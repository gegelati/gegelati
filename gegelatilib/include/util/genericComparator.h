
#ifndef GENERIC_COMPARATOR_H
#define GENERIC_COMPARATOR_H

template <typename T>
/** \brief A comparator for unique_ptr that compares the pointed-to values. */
struct UniqueLess
{

    /**
     * \brief Type alias to signal that this comparator is transparent.
     */
    using is_transparent = void;

    /**
     * \brief Compare two unique_ptr by comparing the pointed-to values.
     */
    bool operator()(const std::unique_ptr<T>& a,
                    const std::unique_ptr<T>& b) const
    {
        return *a < *b;
    }

    /**
     * \brief Compare a unique_ptr and a raw pointer by comparing the
     * pointed-to values.
     */
    bool operator()(const std::unique_ptr<T>& a, const T* b) const
    {
        return *a < *b;
    }

    /**
     * \brief Compare a raw pointer and a unique_ptr by comparing the
     * pointed-to values.
     */
    bool operator()(const T* a, const std::unique_ptr<T>& b) const
    {
        return *a < *b;
    }
};

template <typename T>
/** \brief A comparator for shared_ptr that compares the pointed-to values. */
struct SharedLess
{

    /**
     * \brief Type alias to signal that this comparator is transparent.
     */
    using is_transparent = void;

    /**
     * \brief Compare two shared_ptr by comparing the pointed-to values.
     */
    bool operator()(const std::shared_ptr<T>& a,
                    const std::shared_ptr<T>& b) const
    {
        return *a < *b;
    }

    /**
     * \brief Compare a shared_ptr and a raw pointer by comparing the
     * pointed-to values.
     */
    bool operator()(const std::shared_ptr<T>& a, const T* b) const
    {
        return *a < *b;
    }

    /**
     * \brief Compare a raw pointer and a shared_ptr by comparing the
     * pointed-to values.
     */
    bool operator()(const T* a, const std::shared_ptr<T>& b) const
    {
        return *a < *b;
    }
};

#endif