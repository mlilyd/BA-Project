#ifndef BOUNDINGBOX_H
#define BOUNDINGBOX_H

#include "Renderer.h"

class BoundingBox {
    public:
        BoundingBox() {}
        BoundingBox(const Point3& a, const Point3& b) { _min = a; _max = b;}

        Point3 min() const {return _min; }
        Point3 max() const {return _max; }

        bool hit(const Ray& r, double tmin, double tmax) const {
            for (int a = 0; a < 3; a++) {
                auto invD = 1.0f / r.direction()[a];
                auto t0 = (min()[a] - r.origin()[a]) * invD;
                auto t1 = (max()[a] - r.origin()[a]) * invD;
                if (invD < 0.0f)
                    std::swap(t0, t1);
                tmin = t0 > tmin ? t0 : tmin;
                tmax = t1 < tmax ? t1 : tmax;
                if (tmax <= tmin)
                    return false;
            }
            return true;
        }

    private:
        Point3 _min;
        Point3 _max;
};


BoundingBox surrounding_box(BoundingBox box0, BoundingBox box1){
    Point3 small(fmin(box0.min().x(), box1.min().x()), 
                 fmin(box0.min().y(), box1.min().y()),
                 fmin(box0.min().z(), box1.min().z()));
    
    Point3 big(fmax(box0.max().x(), box1.max().x()), 
               fmax(box0.max().y(), box1.max().y()),
               fmax(box0.max().z(), box1.max().z()));

    return BoundingBox(small, big);
}



#endif