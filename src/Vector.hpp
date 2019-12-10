#ifndef DYNRPG_PATHFINDER_VECTOR_HPP
#define DYNRPG_PATHFINDER_VECTOR_HPP

#pragma once

class Vector
{
public:
    int x = 0;
    int y = 0;
    
    Vector() = default;
    Vector(const Vector&) = default;
    Vector& operator =(const Vector&) = default;

    Vector& operator +=(const Vector& _rhs)
    {
        x += _rhs.x;
        y += _rhs.y;
        return *this;
    }
    
    Vector& operator -=(const Vector& _rhs)
    {
        x -= _rhs.x;
        y -= _rhs.y;
        return *this;
    }
    
    Vector& operator *=(int _value)
    {
        x *= _value;
        y *= _value;
        return *this;
    }
    
    Vector& operator /=(int _value)
    {
        x /= _value;
        y /= _value;
        return *this;
    }
    
    int manhattan_length() const
    {
        return std::abs(x) + std::abs(y);
    }
    
    friend bool operator ==(const Vector& _lhs, const Vector& _rhs)
    {
        return _lhs.x == _rhs.x && _lhs.y == _rhs.y;
    }
    
    friend bool operator !=(const Vector& _lhs, const Vector& _rhs)
    {
        return !(_lhs == _rhs);
    }
    
    friend Vector operator +(Vector _lhs, const Vector& _rhs)
    {
        _lhs += _rhs;
        return _lhs;
    }

    friend Vector operator -(Vector _lhs, const Vector& _rhs)
    {
        _lhs -= _rhs;
        return _lhs;
    }
    
    friend Vector operator *(Vector _lhs, int _value)
    {
        _lhs *= _value;
        return _lhs;
    }

    friend Vector operator /(Vector _lhs, int _value)
    {
        _lhs /= _value;
        return _lhs;
    }
};

#endif
