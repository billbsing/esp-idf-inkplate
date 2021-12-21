#include "PCF85063A.hpp"

// Orignl source from https://github.com/e-radionicacom/PCF85063A-Arduino-Library

// INIT
PCF85063A::PCF85063A(void)
{
}

// PUBLIC
void PCF85063A::setTime(uint8_t hour, uint8_t minute, uint8_t second)
{

    Wire::enter();

	wire.begin_transmission(I2C_ADDR);
	wire.write(RTC_SECOND_ADDR);
	wire.write(decToBcd( second) );
	wire.write(decToBcd( minute) );
	wire.write(decToBcd( hour) );
	wire.end_transmission();

	Wire::leave();
}

void PCF85063A::setDate(uint8_t weekday, uint8_t day, uint8_t month, uint8_t yr)
{
	year = yr - 1970; 	// convert to RTC year format 0-99


    Wire::enter();

	wire.begin_transmission(I2C_ADDR);
	wire.write(RTC_DAY_ADDR);
	wire.write( decToBcd(day) );
	wire.write( decToBcd(weekday) );
	wire.write( decToBcd(month) );
	wire.write( decToBcd(year) );
	wire.end_transmission();

	Wire::leave();

}

void PCF85063A::setDateTime(struct tm *now) {
	setDate(now->tm_wday, now->tm_mday, now->tm_mon, now->tm_year);
	setTime(now->tm_hour, now->tm_min, now->tm_sec);
}

void PCF85063A::readDateTime(struct tm *now) {
	readTime();
	now->tm_wday = weekday;
	now->tm_mday = day;
	now->tm_mon = month;
	now->tm_year = year;
	now->tm_hour = hour;
	now->tm_min = minute;
	now->tm_sec = second;
}

void PCF85063A::readTime()
{

	Wire::enter();
	wire.begin_transmission(I2C_ADDR);
	wire.write(RTC_SECOND_ADDR);					// datasheet 8.4.
	wire.end_transmission();

	wire.request_from(I2C_ADDR, 7);

	while( wire.available() )
	{
		second = bcdToDec( wire.read() & 0x7F ); 	// ignore bit 7
		minute = bcdToDec( wire.read() & 0x7F );
		hour = bcdToDec( wire.read() & 0x3F );		// ignore bits 7 & 6
		day = bcdToDec( wire.read() & 0x3F );
		weekday = bcdToDec( wire.read() & 0x07);	// ignore bits 7,6,5,4 & 3
		month = bcdToDec( wire.read() & 0x1F);		// ignore bits 7,6 & 5
		year = bcdToDec( wire.read()) + 1970;
	}
	Wire::leave();

}

uint8_t PCF85063A::getSecond()
{
	readTime();
	return second;
}

uint8_t PCF85063A::getMinute()
{
	readTime();
	return minute;
}

uint8_t PCF85063A::getHour()
{
	readTime();
	return hour;
}

uint8_t PCF85063A::getDay()
{
	readTime();
	return day;
}

uint8_t PCF85063A::getWeekday()
{
	readTime();
	return weekday;
}

uint8_t PCF85063A::getMonth()
{
	readTime();
	return month;
}

uint16_t PCF85063A::getYear()
{
	readTime();
	return year;
}

void PCF85063A::enableAlarm() // datasheet 8.5.6.
{
	// check Table 2. Control_2
	control_2 = RTC_CTRL_2_DEFAULT | RTC_ALARM_AIE; 	// enable interrupt
	control_2 &= ~RTC_ALARM_AF;							// clear alarm flag

    Wire::enter();

	wire.begin_transmission(I2C_ADDR);
	wire.write(RTC_CTRL_2);
	wire.write(control_2);
	wire.end_transmission();

	Wire::leave();

}

void PCF85063A::setAlarm(int8_t alarm_second, int8_t alarm_minute, int8_t alarm_hour, int8_t alarm_day, int8_t alarm_weekday)
{
	if (alarm_second >= 0) {										// second
        alarm_second = constrain(alarm_second, 0, 59);
        alarm_second = decToBcd(alarm_second);
        alarm_second &= ~RTC_ALARM;
    } else {
        alarm_second = 0x0; alarm_second |= RTC_ALARM;
    }

	if (alarm_minute >= 0) {										// minute
        alarm_minute = constrain(alarm_minute, 0, 59);
        alarm_minute = decToBcd(alarm_minute);
        alarm_minute &= ~RTC_ALARM;
    } else {
        alarm_minute = 0x0; alarm_minute |= RTC_ALARM;
    }

    if (alarm_hour >= 0) {										// hour
        alarm_hour = constrain(alarm_hour, 0, 23);
        alarm_hour = decToBcd(alarm_hour);
        alarm_hour &= ~RTC_ALARM;
    } else {
        alarm_hour = 0x0; alarm_hour |= RTC_ALARM;
    }

    if (alarm_day >= 0) {										// day
        alarm_day = constrain(alarm_day, 1, 31);
        alarm_day = decToBcd(alarm_day);
        alarm_day &= ~RTC_ALARM;
    } else {
        alarm_day = 0x0; alarm_day |= RTC_ALARM;
    }

    if (alarm_weekday >= 0) {									// weekday
        alarm_weekday = constrain(alarm_weekday, 0, 6);
        alarm_weekday = decToBcd(alarm_weekday);
        alarm_weekday &= ~RTC_ALARM;
    } else {
        alarm_weekday = 0x0; alarm_weekday |= RTC_ALARM;
    }

    enableAlarm();
	Wire::enter();

    wire.begin_transmission(I2C_ADDR);
    wire.write(RTC_SECOND_ALARM);
    wire.write(alarm_second);
    wire.write(alarm_minute);
    wire.write(alarm_hour);
    wire.write(alarm_day);
    wire.write(alarm_weekday);
    wire.end_transmission();

	Wire::leave();

}

void PCF85063A::readAlarm()
{
	Wire::enter();

	wire.begin_transmission(I2C_ADDR);
	wire.write(RTC_SECOND_ALARM);					// datasheet 8.4.
	wire.end_transmission();

	wire.request_from(I2C_ADDR, 5);

	while( wire.available() )
	{
		alarm_second = wire.read(); 				// read RTC_SECOND_ALARM register
		if( RTC_ALARM &  alarm_second)				// check is AEN = 1 (second alarm disabled)
		{
			alarm_second = 99;						// using 99 as code for no alarm
		} else {									// else if AEN = 0 (second alarm enabled)
			alarm_second = bcdToDec( alarm_second & ~RTC_ALARM);	// remove AEN flag and convert to dec number
		}

		alarm_minute = wire.read(); 				// minute
		if( RTC_ALARM &  alarm_minute)
		{
			alarm_minute = 99;
		} else {
			alarm_minute = bcdToDec( alarm_minute & ~RTC_ALARM);
		}

		alarm_hour = wire.read(); 				// hour
		if( RTC_ALARM &  alarm_hour)
		{
			alarm_hour = 99;
		} else {
			alarm_hour = bcdToDec( alarm_hour & 0x3F);	// remove bits 7 & 6
		}

		alarm_day = wire.read(); 				// day
		if( RTC_ALARM &  alarm_day)
		{
			alarm_day = 99;
		} else {
			alarm_day = bcdToDec( alarm_day & 0x3F);	// remove bits 7 & 6
		}

		alarm_weekday = wire.read(); 				// weekday
		if( RTC_ALARM &  alarm_weekday)
		{
			alarm_weekday = 99;
		} else {
			alarm_weekday = bcdToDec( alarm_weekday & 0x07);	// remove bits 7,6,5,4 & 3
		}
	}
	Wire::leave();
}

uint8_t PCF85063A::getAlarmSecond()
{
	readAlarm();
	return alarm_second;
}

uint8_t PCF85063A::getAlarmMinute()
{
	readAlarm();
	return alarm_minute;
}

uint8_t PCF85063A::getAlarmHour()
{
	readAlarm();
	return alarm_hour;
}

uint8_t PCF85063A::getAlarmDay()
{
	readAlarm();
	return alarm_day;
}

uint8_t PCF85063A::getAlarmWeekday()
{
	readAlarm();
	return alarm_weekday;
}

void PCF85063A::timerSet(CountdownSrcClock source_clock, uint8_t value, bool int_enable, bool int_pulse)
{
	uint8_t timer_reg[2] = {0};

	Wire::enter();

	// disable the countdown timer
	wire.begin_transmission(I2C_ADDR);
	wire.write(RTC_TIMER_MODE);
	wire.write(0x18);	// default
	wire.end_transmission();

	// clear Control_2
	wire.begin_transmission(I2C_ADDR);
	wire.write(RTC_CTRL_2);
	wire.write(0x00);	// default
	wire.end_transmission();

	// reconfigure timer
	timer_reg[1] |= RTC_TIMER_TE;						// enable timer
	if (int_enable) timer_reg[1] |= RTC_TIMER_TIE;		// enable interrupr
	if (int_pulse) timer_reg[1] |= RTC_TIMER_TI_TP;		// interrupt mode
	timer_reg[1] |= source_clock << 3;					// clock source
	//timer_reg[1] = 0b00011111;

	timer_reg[0] = value;

	// write timer value
	wire.begin_transmission(I2C_ADDR);
	wire.write(RTC_TIMER_VAL);
	wire.write(timer_reg[0]);
	wire.write(timer_reg[1]);
	wire.end_transmission();

	Wire::leave();

}

bool PCF85063A::checkTimerFlag()
{
	uint8_t _crtl_2 = RTC_TIMER_FLAG;

	Wire::enter();

	wire.begin_transmission(I2C_ADDR);
	wire.write(RTC_CTRL_2);
	wire.end_transmission();
	wire.request_from(I2C_ADDR, 1);
	_crtl_2 &= wire.read();

	Wire::leave();

	return _crtl_2;
}

void PCF85063A::reset()	// datasheet 8.2.1.3.
{
	Wire::enter();

	wire.begin_transmission(I2C_ADDR);
	wire.write(RTC_CTRL_1);
	wire.write(0x58);
	wire.end_transmission();

	Wire::leave();

}


// PRIVATE
uint8_t PCF85063A::decToBcd(uint8_t val)
{
  return ( (val/10*16) + (val%10) );
}

uint8_t PCF85063A::bcdToDec(uint8_t val)
{
  return ( (val/16*10) + (val%16) );
}

uint8_t PCF85063A::constrain(uint8_t value, uint8_t min, uint8_t max) {
	if (value < min) {
		value = min;
	}
	if (value > max) {
		value = max;
	}
	return value;
}
