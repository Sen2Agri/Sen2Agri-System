template <typename TInput, typename TOutput>
class CreateMaskFromValueFunctor
{
public:
    typedef typename TInput::ValueType InputValueType;

    CreateMaskFromValueFunctor() : m_NoDataValue()
    {
    }

    void SetNoDataValue(InputValueType value)
    {
        m_NoDataValue = value;
    }

    InputValueType GetNoDataValue() const
    {
        return m_NoDataValue;
    }

    TOutput operator()(const TInput &in) const
    {
        auto size = in.GetSize();
        for (decltype(size) i = 0; i < size; i++) {
            if (in[i] != m_NoDataValue) {
                return 1;
            }
        }

        return 0;
    }

private:
    InputValueType m_NoDataValue;
};
