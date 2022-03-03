#include "swerveModule.h"
#include "hardwareSettings.h"
#include <ctre/Phoenix.h>

#include <iostream>
using namespace drivetrainConstants::calculations;

swerveModule::swerveModule(const int module[])
    : m_motorDrive(module[0]), m_motorTurn(module[1]), m_encoderTurn(module[2]) {
    std::cout << "\nFor module #" << module[0] << ":\n";
    std::cout << m_motorDrive.GetDeviceID() << " - drive Falcon ID.\n";
    std::cout << m_motorTurn.GetDeviceID() << " - turn Falcon ID.\n";
    std::cout << m_encoderTurn.GetDeviceNumber() << " - CANCoder for steering.\n\n";
    ConfigModule(ConfigType::motorDrive);
    ConfigModule(ConfigType::motorTurn);
    ConfigModule(ConfigType::encoderTurn);
}

void swerveModule::ConfigModule(const ConfigType& type) {
    switch(type) {
        case ConfigType::motorDrive :
            m_motorDrive.ConfigFactoryDefault();
            m_motorDrive.ConfigAllSettings(m_settings.motorDrive);
            //Bevel gear on left side.
            m_motorDrive.SetInverted(ctre::phoenix::motorcontrol::TalonFXInvertType::Clockwise);
            break;
        case ConfigType::motorTurn :
            m_motorTurn.ConfigFactoryDefault();
            m_motorTurn.ConfigRemoteFeedbackFilter(m_encoderTurn.GetDeviceNumber(),
                                                   ctre::phoenix::motorcontrol::
                                                   RemoteSensorSource::RemoteSensorSource_CANCoder, 0, 0);
            //m_motorTurn.ConfigAllSettings(m_settings.motorTurn);
            m_motorTurn.SetInverted(ctre::phoenix::motorcontrol::TalonFXInvertType::CounterClockwise);
            break;
        case ConfigType::encoderTurn :
            m_encoderTurn.ConfigFactoryDefault();
            m_encoderTurn.ConfigAllSettings(m_settings.encoderTurn);
            break;
        default :
            throw std::invalid_argument("Invalid swerveModule ConfigType");
    }
}

frc::SwerveModuleState swerveModule::GetState() {
    units::native_units_per_decisecond_t motorSpeed{m_motorDrive.GetSelectedSensorVelocity(0)};
    units::meters_per_second_t wheelSpeed{(motorSpeed * wheelCircumference) / finalDriveRatio};
    return {wheelSpeed,frc::Rotation2d(units::degree_t(m_encoderTurn.GetAbsolutePosition()))};
}

void swerveModule::SetDesiredState(const frc::SwerveModuleState& referenceState) {
    const auto state = frc::SwerveModuleState::Optimize(
        referenceState,units::degree_t(m_encoderTurn.GetAbsolutePosition()));
        std::cout << m_encoderTurn.GetAbsolutePosition() << "-abs_pos\n";

        const auto targetWheelSpeed{state.speed};
        const auto targetAngle{state.angle};

        units::native_units_per_decisecond_t targetMotorSpeed{
            (targetWheelSpeed * finalDriveRatio) / wheelCircumference};
        m_motorDrive.Set(ctre::phoenix::motorcontrol::ControlMode::Velocity, targetMotorSpeed.value());

        m_motorTurn.Set(ctre::phoenix::motorcontrol::ControlMode::MotionMagic, targetAngle.Degrees().value());
        if (state.angle.Degrees().value() == targetAngle.Degrees().value()) {
            //std::cout << state.angle.Degrees().value();
            std::cout << targetAngle.Degrees().value() << "-target_Deg\n";
        } else {std::cout << "Fuck.";}

}