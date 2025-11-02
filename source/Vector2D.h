#pragma once
#include <iostream>
#include <cmath>

class Vector2D {
public:
    Vector2D();
    Vector2D(float x, float y);

    float getX() const;
    void setX(float x);
    float getY() const;
    void setY(float y);
    Vector2D lerp(const Vector2D& target, float t) const;

    float magnitude() const;
    Vector2D normalized() const;
    Vector2D operator+(const Vector2D& other) const;
    Vector2D operator-(const Vector2D& other) const;
    Vector2D operator*(float scalar) const;
    Vector2D operator/(float scalar) const;
    bool operator==(const Vector2D& other) const;
    bool operator!=(const Vector2D& other) const;
    bool operator<(const Vector2D& other) const;

    float distanceManhattan(const Vector2D& other)const;
    float distanceEuclidean(const Vector2D& other)const;
    float distanceSquared(const Vector2D& other)const;

    friend std::ostream& operator<<(std::ostream& os, const Vector2D& vec);

private:
    float x;
    float y;
};
