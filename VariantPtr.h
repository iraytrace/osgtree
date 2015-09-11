#ifndef VARIANPTR_H
#define VARIANPTR_H
#include <QVariant>

template <class T> class VariantPtr
{

public:

    static T* asPtr(QVariant v)
    {
        return  (T *) v.value<void *>();
    }

    static QVariant asQVariant(T* ptr)
    {
        return QVariant::fromValue((void *) ptr);
    }
};
#endif // VARIANPTR_H
