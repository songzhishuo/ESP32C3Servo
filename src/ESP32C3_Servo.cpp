#include "ESP32C3_Servo.h"
// #include "esp32-hal-ledc.h"
#include "Arduino.h"

// 舵机数量计数器
int Servo::ServoCount = 0;

// ChannelUse 数组元素如果从未使用过，则为 0，如果使用中，则为 1，如果释放，则为 -1
int Servo::ChannelUsed[MAX_SERVOS + 1] = {0}; // 忽略0号元素

Servo::Servo()
{
    this->servoChannel = 0; // 绑定的通道号
    // 查找空闲通道
    bool foundChannelForReuse = false;
    for (int i = 1; i < MAX_SERVOS + 1; i++)
    {
        if (ChannelUsed[i] == -1) // 找到回收的通道
        {
            // reclaim this channel
            ChannelUsed[i] = 1;
            this->servoChannel = i;
            foundChannelForReuse = true; // 找到可用通道
            break;
        }
    }
    if (!foundChannelForReuse) // 未找到可回收通道
    {
        // 没找到可重复利用的通道
        if (ServoCount < MAX_SERVOS)
        {
            this->servoChannel = ++ServoCount; // assign a servo channel number to this instance
            ChannelUsed[this->servoChannel] = 1;
        }
        else
        {
            this->servoChannel = 0; // too many servos in use
        }
    }
    // 如果我们得到一个通道，完成初始化
    if (this->servoChannel > 0)
    {
        // initialize this channel with plausible values, except pin # (we set pin # when attached)
        this->ticks = DEFAULT_PULSE_WIDTH_TICKS;
        this->timer_width = DEFAULT_TIMER_WIDTH;
        this->pinNumber = -1; // make it clear that we haven't attached a pin to this channel
        this->min = DEFAULT_uS_LOW;
        this->max = DEFAULT_uS_HIGH;
        this->timer_width_ticks = pow(2, this->timer_width);
    }
}

int Servo::attach(int pin)
{
    return (this->attach(pin, DEFAULT_uS_LOW, DEFAULT_uS_HIGH));
}

int Servo::attach(int pin, int min, int max)
{
    if ((this->servoChannel <= MAX_SERVOS) && (this->servoChannel > 0))
    {
        // OK to proceed; first check for new/reuse
        if (this->pinNumber < 0) // we are attaching to a new or previously detached pin; we need to initialize/reinitialize
        {
            // claim/reclaim this channel
            ChannelUsed[this->servoChannel] = 1;
            this->ticks = DEFAULT_PULSE_WIDTH_TICKS;
            this->timer_width = DEFAULT_TIMER_WIDTH;
            this->timer_width_ticks = pow(2, this->timer_width);
        }
        this->pinNumber = pin;

        // min/max checks
        if (min < MIN_PULSE_WIDTH) // ensure pulse width is valid
            min = MIN_PULSE_WIDTH;
        if (max > MAX_PULSE_WIDTH)
            max = MAX_PULSE_WIDTH;
        this->min = min; // store this value in uS
        this->max = max; // store this value in uS
        // Set up this channel
        // if you want anything other than default timer width, you must call setTimerWidth() before attach
        ledcSetup(this->servoChannel, REFRESH_CPS, this->timer_width); // channel #, 50 Hz, timer width
        ledcAttachPin(this->pinNumber, this->servoChannel);            // GPIO pin assigned to channel
    }
    else
        return 0;
}

void Servo::detach()
{
    if (this->attached())
    {
        ledcDetachPin(this->pinNumber);
        // keep track of detached servos channels so we can reuse them if needed
        ChannelUsed[this->servoChannel] = -1;
        this->pinNumber = -1;
    }
}

void Servo::write(int value)
{
    // treat values less than MIN_PULSE_WIDTH (500) as angles in degrees (valid values in microseconds are handled as microseconds)
    if (value < MIN_PULSE_WIDTH)
    {
        if (value < 0)
            value = 0;
        else if (value > 180)
            value = 180;

        value = map(value, 0, 180, this->min, this->max);
    }
    this->writeMicroseconds(value);
}

void Servo::writeMicroseconds(int value)
{
    // calculate and store the values for the given channel
    if ((this->servoChannel <= MAX_SERVOS) && (this->attached())) // ensure channel is valid
    {
        if (value < this->min) // ensure pulse width is valid
            value = this->min;
        else if (value > this->max)
            value = this->max;

        value = usToTicks(value); // convert to ticks
        this->ticks = value;
        // do the actual write
        ledcWrite(this->servoChannel, this->ticks);
    }
}

int Servo::read() // return the value as degrees
{
    return (map(readMicroseconds() + 1, this->min, this->max, 0, 180));
}

int Servo::readMicroseconds()
{
    int pulsewidthUsec;
    if ((this->servoChannel <= MAX_SERVOS) && (this->attached()))
    {
        pulsewidthUsec = ticksToUs(this->ticks);
    }
    else
    {
        pulsewidthUsec = 0;
    }

    return (pulsewidthUsec);
}

bool Servo::attached()
{
    return (ChannelUsed[this->servoChannel]);
}

void Servo::setTimerWidth(int value)
{
    // only allow values between 16 and 20
    if (value < 16)
        value = 16;
    else if (value > 20)
        value = 20;

    // Fix the current ticks value after timer width change
    // The user can reset the tick value with a write() or writeUs()
    int widthDifference = this->timer_width - value;
    // if positive multiply by diff; if neg, divide
    if (widthDifference > 0)
    {
        this->ticks << widthDifference;
    }
    else
    {
        this->ticks >> widthDifference;
    }

    this->timer_width = value;
    this->timer_width_ticks = pow(2, this->timer_width);

    // If this is an attached servo, clean up
    if ((this->servoChannel <= MAX_SERVOS) && (this->attached()))
    {
        // detach, setup and attach again to reflect new timer width
        ledcDetachPin(this->pinNumber);
        ledcSetup(this->servoChannel, REFRESH_CPS, this->timer_width);
        ledcAttachPin(this->pinNumber, this->servoChannel);
    }
}

int Servo::readTimerWidth()
{
    return (this->timer_width);
}

int Servo::usToTicks(int usec)
{
    return (int)((float)usec / ((float)REFRESH_USEC / (float)this->timer_width_ticks));
}

int Servo::ticksToUs(int ticks)
{
    return (int)((float)ticks * ((float)REFRESH_USEC / (float)this->timer_width_ticks));
}
