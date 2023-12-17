class RotatePlayer
{
public:
    RotatePlayer(int rotate_left_pin, int rotate_right_pin, int rotate_step)
        : m_rotate_left_pin(rotate_left_pin), m_rotate_right_pin(rotate_right_pin), m_rotate_step(rotate_step)
    {
    }

    int getSegments()
    {
        int leftValue = analogRead(m_rotate_left_pin);
        int rightValue = analogRead(m_rotate_right_pin);

        int leftSegment = leftValue / m_rotate_step;
        int rightSegment = rightValue / m_rotate_step;

        return (leftSegment << 4) | rightSegment;
    }

    int getBitShift()
    {
        return m_bit_shift;
    }

private:
    int m_rotate_left_pin;
    int m_rotate_right_pin;
    int m_rotate_step;
    int m_bit_shift;
};
