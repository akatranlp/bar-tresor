// Touch Level

class TouchPlayer
{
public:
    enum class Touch : u8
    {
        None = 0,
        Up = 1,
        Back = 2,
        Left = 4,
        Right = 8,
    };

#define TOUCH_NONE static_cast<u8>(TouchPlayer::Touch::None)
#define TOUCH_UP static_cast<u8>(TouchPlayer::Touch::Up)
#define TOUCH_BACK static_cast<u8>(TouchPlayer::Touch::Back)
#define TOUCH_LEFT static_cast<u8>(TouchPlayer::Touch::Left)
#define TOUCH_RIGHT static_cast<u8>(TouchPlayer::Touch::Right)

    TouchPlayer(int up_pin, int back_pin, int left_pin, int right_pin) : m_up_pin(up_pin), m_back_pin(back_pin), m_left_pin(left_pin), m_right_pin(right_pin)
    {
        pinMode(m_up_pin, INPUT);
        pinMode(m_back_pin, INPUT);
        pinMode(m_left_pin, INPUT);
        pinMode(m_right_pin, INPUT);
    }

    u8 getTouchInput()
    {
        u8 touch = 0;
        if (digitalRead(m_up_pin))
        {
            touch |= TOUCH_UP;
        }
        if (digitalRead(m_back_pin))
        {
            touch |= TOUCH_BACK;
        }
        if (digitalRead(m_left_pin))
        {
            touch |= TOUCH_LEFT;
        }
        if (digitalRead(m_right_pin))
        {
            touch |= TOUCH_RIGHT;
        }
        return touch;
    }

private:
    int m_up_pin;
    int m_back_pin;
    int m_left_pin;
    int m_right_pin;
};
