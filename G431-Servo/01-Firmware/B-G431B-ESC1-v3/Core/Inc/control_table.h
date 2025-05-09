/*
 * control_table.h
 *
 *  Created on: 26 sept. 2020
 *      Author: patrick
 */

#ifndef INC_CONTROL_TABLE_H_
#define INC_CONTROL_TABLE_H_

#include "stm32g4xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

// REGISTER ADDRESS ///////////////////////////////////////////////////////////

// EEPROM
#define REG_MODEL_NUMBER_L 						0x00 	//0
#define REG_MODEL_NUMBER_H 						0x01	//1
#define REG_VERSION		 						0x02	//2
#define REG_ID             						0x03	//3
#define REG_BAUD_RATE      						0x04	//4
#define REG_RETURN_DELAY   						0x05	//5 // RESERVED

#define REG_MIN_POSITION_DEG_L					0x10	//16
#define REG_MIN_POSITION_DEG_H					0x11	//17
#define REG_MAX_POSITION_DEG_L					0x12	//18
#define REG_MAX_POSITION_DEG_H					0x13	//19
#define REG_MAX_VELOCITY_DPS_L					0x14	//20
#define REG_MAX_VELOCITY_DPS_H					0x15	//21
#define REG_MAX_ACCELERATION_DPSS_L				0x16	//22 // RESERVED
#define REG_MAX_ACCELERATION_DPSS_H				0x17	//23 // RESERVED
#define REG_MAX_CURRENT_MA_L					0x18	//24
#define REG_MAX_CURRENT_MA_H					0x19	//25
#define REG_RESERVED_RAM_1						0x1A	//26 // RESERVED
#define REG_RESERVED_RAM_2						0x1B	//27 // RESERVED
#define REG_TEMPERATURE_LIMIT					0x1C	//28
#define REG_LOW_VOLTAGE_LIMIT					0x1D	//29
#define REG_HIGH_VOLTAGE_LIMIT					0x1E	//30

#define REG_MOVING_THRESHOLD_DPS				0x1F	//31
#define REG_STATUS_RETURN_LVL					0x20	//32 // RESERVED
#define REG_ALARM_LED							0x21	//33 // RESERVED
#define REG_ALARM_SHUTDOWN						0x22	//34 // RESERVED

#define REG_ENCODER_BITS						0x23	//35
#define REG_MOTOR_POLE_PAIRS					0x24	//36
#define REG_MOTOR_SYNCHRO_L						0x25	//37
#define REG_MOTOR_SYNCHRO_H						0x26	//38
#define REG_RESERVED_RAM_3						0x27	//39 // RESERVED
#define REG_INV_PHASE_MOTOR		    			0x28	//40
#define REG_FIELD_WEAKENING_K	 				0x29	//41

#define REG_PID_POSITION_KP_L					0x2A	//42
#define REG_PID_POSITION_KP_H					0x2B	//43
#define REG_PID_POSITION_KI_L					0x2C	//44 // RESERVED
#define REG_PID_POSITION_KI_H					0x2D	//45 // RESERVED
#define REG_PID_POSITION_KD_L					0x2E	//46
#define REG_PID_POSITION_KD_H					0x2F	//47

#define REG_PID_VELOCITY_KP_L					0x30	//48 // RESERVED
#define REG_PID_VELOCITY_KP_H					0x31	//49 // RESERVED
#define REG_PID_VELOCITY_KI_L					0x32	//50 // RESERVED
#define REG_PID_VELOCITY_KI_H					0x33	//51 // RESERVED
#define REG_PID_VELOCITY_KD_L					0x34	//52 // RESERVED
#define REG_PID_VELOCITY_KD_H					0x35	//53 // RESERVED
#define REG_PID_VELOCITY_KFF_L					0x36	//54 // RESERVED
#define REG_PID_VELOCITY_KFF_H					0x37	//55 // RESERVED
#define REG_PID_ACCELERATION_KFF_L				0x38	//56 // RESERVED
#define REG_PID_ACCELERATION_KFF_H				0x39	//57 // RESERVED

#define REG_PID_FLUX_CURRENT_KP_L				0x3A	//58
#define REG_PID_FLUX_CURRENT_KP_H				0x3B	//59
#define REG_PID_FLUX_CURRENT_KI_L				0x3C	//60 // RESERVED
#define REG_PID_FLUX_CURRENT_KI_H				0x3D	//61 // RESERVED
#define REG_PID_FLUX_CURRENT_KFF_L				0x3E	//62 // RESERVED
#define REG_PID_FLUX_CURRENT_KFF_H				0x3F	//63 // RESERVED

#define REG_PID_TORQUE_CURRENT_KP_L				0x40	//64
#define REG_PID_TORQUE_CURRENT_KP_H				0x41	//65
#define REG_PID_TORQUE_CURRENT_KI_L				0x42	//66 // RESERVED
#define REG_PID_TORQUE_CURRENT_KI_H				0x43	//67 // RESERVED
#define REG_PID_TORQUE_CURRENT_KFF_L			0x44	//68 // RESERVED
#define REG_PID_TORQUE_CURRENT_KFF_H			0x45	//69 // RESERVED

#define REG_CAL_PHASE1_CURRENT_SENSE_MA_L		0x46	//70 // RESERVED
#define REG_CAL_PHASE1_CURRENT_SENSE_MA_H		0x47	//71 // RESERVED
#define REG_CAL_PHASE1_CURRENT_SENSE_OFFSET_L	0x48	//72 // RESERVED
#define REG_CAL_PHASE1_CURRENT_SENSE_OFFSET_H	0x49	//73 // RESERVED

#define REG_CAL_PHASE2_CURRENT_SENSE_MA_L		0x4A	//74 // RESERVED
#define REG_CAL_PHASE2_CURRENT_SENSE_MA_H		0x4B	//75 // RESERVED
#define REG_CAL_PHASE2_CURRENT_SENSE_OFFSET_L	0x4C	//76 // RESERVED
#define REG_CAL_PHASE2_CURRENT_SENSE_OFFSET_H	0x4D	//77 // RESERVED

#define REG_CAL_PHASE3_CURRENT_SENSE_MA_L		0x4E	//78 // RESERVED
#define REG_CAL_PHASE3_CURRENT_SENSE_MA_H		0x4F	//79 // RESERVED
#define REG_CAL_PHASE3_CURRENT_SENSE_OFFSET_L	0x50	//80 // RESERVED
#define REG_CAL_PHASE3_CURRENT_SENSE_OFFSET_H	0x51	//81 // RESERVED

#define REG_CAL_VOLTAGE_SENSOR_L				0x52	//82 // RESERVED
#define REG_CAL_VOLTAGE_SENSOR_H				0x53	//83 // RESERVED

#define REG_EWMA_ENCODER						0x54	//84

// RAM
#define REG_TORQUE_ENABLE  						0x80 //0
#define REG_LED			  						0x81 //1
#define REG_CONTROL_MODE						0x82 //2
#define REG_GOAL_POSITION_DEG_L					0x83 //3
#define REG_GOAL_POSITION_DEG_H					0x84 //4
#define REG_GOAL_VELOCITY_DPS_L					0x85 //5
#define REG_GOAL_VELOCITY_DPS_H					0x86 //6
#define REG_GOAL_TORQUE_CURRENT_MA_L			0x87 //7
#define REG_GOAL_TORQUE_CURRENT_MA_H			0x88 //8
#define REG_GOAL_FLUX_CURRENT_MA_L				0x89 //9
#define REG_GOAL_FLUX_CURRENT_MA_H				0x8A //10
#define REG_GOAL_POS_KP							0x8B //11
#define REG_GOAL_POS_KD							0x8C //12
#define REG_GOAL_VEL_KP							0x8D //13
#define REG_GOAL_SYNCHRO_OFFSET_L				0x8E //14
#define REG_GOAL_SYNCHRO_OFFSET_H				0x8F //15

#define REG_PRESENT_POSITION_DEG_L				0x90 //16
#define REG_PRESENT_POSITION_DEG_H				0x91 //17
#define REG_PRESENT_VELOCITY_DPS_L				0x92 //18
#define REG_PRESENT_VELOCITY_DPS_H				0x93 //19
#define REG_PRESENT_TORQUE_CURRENT_MA_L			0x94 //20
#define REG_PRESENT_TORQUE_CURRENT_MA_H			0x95 //21
#define REG_PRESENT_FLUX_CURRENT_MA_L			0x96 //22
#define REG_PRESENT_FLUX_CURRENT_MA_H			0x97 //23
#define REG_PRESENT_VOLTAGE						0x98 //24
#define REG_PRESENT_TEMPERATURE 				0x99 //25
#define REG_MOVING			 					0x9A //26
// DEBUG RAM
#define REG_SETPOINT_POSITION_DEG_L				0xA0 //32
#define REG_SETPOINT_POSITION_DEG_H				0xA1 //33
#define REG_SETPOINT_VELOCITY_DPS_L				0xA2 //34
#define REG_SETPOINT_VELOCITY_DPS_H				0xA3 //35
#define REG_SETPOINT_TORQUE_CURRENT_MA_L		0xA4 //36
#define REG_SETPOINT_TORQUE_CURRENT_MA_H		0xA5 //37
#define REG_SETPOINT_FLUX_CURRENT_MA_L			0xA6 //38
#define REG_SETPOINT_FLUX_CURRENT_MA_H			0xA7 //39
#define REG_RESERVED_RAM_5						0xA8 //40
#define REG_RESERVED_RAM_6						0xA9 //41
#define REG_PROCESSING_TIME 					0xAA //42
#define REG_FOC_FREQUENCY	 					0xAB //43
#define REG_PID_FREQUENCY	 					0xAC //44
#define REG_MLP_FREQUENCY	 					0xAD //45

// SW & HW ERROR
#define REG_PROTOCOL_CRC_FAIL 					0xB0 //48
#define REG_HARDWARE_ERROR_STATUS 				0xB1 //49

#define REG_MAX             					0xB2


// REGISTER FACTORY DEFAULT VALUES ////////////////////////////////////////////

#define REG_MODEL_NUMBER_VALUE 				92		//
#define REG_VERSION_VALUE 					30		// FOC V3.0
#define REG_ID_VALUE 						1		// Default ID
#define REG_BAUD_RATE_VALUE 				3		// 1: 1Mbps
#define REG_RETURN_DELAY_VALUE 				0		// 0: zero delay

#define REG_MIN_POSITION_DEG_VALUE 			-800   	// deg
#define REG_MAX_POSITION_DEG_VALUE 			800		// deg
#define REG_MAX_VELOCITY_DPS_VALUE			16000	// dps
#define REG_MAX_ACCELERATION_DPSS_VALUE		0		// dpss
#define REG_MAX_CURRENT_MA_VALUE			26000	// mA

#define REG_TEMPERATURE_LIMIT_VALUE		 	70		// °C
#define REG_LOW_VOLTAGE_LIMIT_VALUE		 	10		// V
#define REG_HIGH_VOLTAGE_LIMIT_VALUE		28		// V

#define REG_MOVING_THRESHOLD_DPS_VALUE		5		// dps
#define REG_STATUS_RETURN_LVL_VALUE			0		// TODO : fill comment
#define REG_ALARM_LED_VALUE					0		// TODO : fill comment
#define REG_ALARM_SHUTDOWN_VALUE			0		// TODO : fill comment

#define REG_ENCODER_BITS_VALUE				14		//
#define REG_MOTOR_POLE_PAIRS_VALUE			14		//
#define REG_MOTOR_SYNCHRO_VALUE				0		// deg
#define REG_INV_PHASE_VALUE					0		// 0: NORMAL 1: INV
#define REG_FIELD_WEAKENING_K_VALUE			255		// (1/)255 is minimum value, recommended around (1/)5

#define REG_PID_POSITION_KP_VALUE			0		// k x1
#define REG_PID_POSITION_KI_VALUE			0		// k x100
#define REG_PID_POSITION_KD_VALUE			0		// k x1

#define REG_PID_VELOCITY_KP_VALUE			0		// k x1000
#define REG_PID_VELOCITY_KI_VALUE			0		// k x100000
#define REG_PID_VELOCITY_KD_VALUE			0		// k x1000
#define REG_PID_VELOCITY_KFF_VALUE			0		// k x1000

#define REG_PID_ACCELERATION_KFF_VALUE		0		// k x1000

#define REG_PID_FLUX_CURRENT_KP_VALUE		10		// k x100000
#define REG_PID_FLUX_CURRENT_KI_VALUE		10		// k x10000000
#define REG_PID_FLUX_CURRENT_KFF_VALUE		0		// k x100000

#define REG_PID_TORQUE_CURRENT_KP_VALUE		10		// k x100000
#define REG_PID_TORQUE_CURRENT_KI_VALUE		10		// k x10000000
#define REG_PID_TORQUE_CURRENT_KFF_VALUE	0		// k x100000

#define REG_CAL_PHASE1_CURRENT_SENSE_MA_VALUE	29	// mA x1000
#define REG_CAL_PHASE2_CURRENT_SENSE_MA_VALUE	29	// mA x1000
#define REG_CAL_PHASE3_CURRENT_SENSE_MA_VALUE	29	// mA x1000

#define REG_CAL_PHASE1_CURRENT_SENSE_OFFSET_VALUE 2500 // ADC
#define REG_CAL_PHASE2_CURRENT_SENSE_OFFSET_VALUE 2500 // ADC
#define REG_CAL_PHASE3_CURRENT_SENSE_OFFSET_VALUE 2500 // ADC

#define REG_CAL_VOLTAGE_SENSOR_VALUE 1000 // V

#define REG_EWMA_ENCODER_VALUE 255

// REGISTER ERROR VALUES //////////////////////////////////////////////////////

#define HW_ERROR_BIT_VOLTAGE 0
#define HW_ERROR_BIT_POSITION_SENSOR_STATUS_ERROR 1
#define HW_ERROR_BIT_POSITION_SENSOR_NOT_RESPONDING 2
#define HW_ERROR_BIT_POSITION_SENSOR_TIMESTAMP 3
#define HW_ERROR_BIT_FOC_TIMEOUT 4
#define HW_ERROR_BIT_OVERLOAD 5
#define HW_ERROR_BIT_OVERHEATING 6

// REGISTER OPERATIONS ////////////////////////////////////////////////////////
void factory_reset_eeprom_regs();
void load_eeprom_regs();
void store_eeprom_regs();
void reset_ram_regs();

extern uint8_t regs[REG_MAX];


#ifdef __cplusplus
}
#endif

#endif /* INC_CONTROL_TABLE_H_ */
