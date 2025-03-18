#pragma once

// Простой класс для представления диапазона
template <typename TIterator>
class TSubrange {
public:
    TSubrange(TIterator begin, TIterator end) : Begin_(begin), End_(end) {}

    TIterator begin() const { return Begin_; }
    TIterator end() const { return End_; }
    bool empty() const { return Begin_ == End_; };

    bool operator==(const TSubrange& other) const { return Begin_ == other.Begin_ && End_ == other.End_; }

private:
    TIterator Begin_;
    TIterator End_;
};

template <class TIterator>
class TIteratorRange {
    public:
        TIteratorRange(TIterator begin, TIterator end)
            : Begin_(begin)
            , End_(end)
        {}

        TIterator begin() const { return Begin_; }
        TIterator end() const { return End_; }
        bool empty() const { return Begin_ == End_; }

    private:
        TIterator Begin_;
        TIterator End_;
};

template <class TIterator>
TIteratorRange<TIterator> MakeIteratorRange(TIterator begin, TIterator end) {
    return TIteratorRange<TIterator>(begin, end);
}