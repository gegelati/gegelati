/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2025) :
 *
 * Quentin Vacher <qvacher@insa-rennes.fr> (2025)
 *
 * GEGELATI is an open-source reinforcement learning framework for training
 * artificial intelligence based on Tangled Program Graphs (TPGs).
 *
 * This software is governed by the CeCILL-C license under French law and
 * abiding by the rules of distribution of free software. You can use,
 * modify and/ or redistribute the software under the terms of the CeCILL-C
 * license as circulated by CEA, CNRS and INRIA at the following URL
 * "http://www.cecill.info".
 *
 * As a counterpart to the access to the source code and rights to copy,
 * modify and redistribute granted by the license, users are provided only
 * with a limited warranty and the software's author, the holder of the
 * economic rights, and the successive licensors have only limited
 * liability.
 *
 * In this respect, the user's attention is drawn to the risks associated
 * with loading, using, modifying and/or developing or reproducing the
 * software by the user in light of its specific status of free software,
 * that may mean that it is complicated to manipulate, and that also
 * therefore means that it is reserved for developers and experienced
 * professionals having in-depth computer knowledge. Users are therefore
 * encouraged to load and test the software's suitability as regards their
 * requirements in conditions enabling the security of their systems and/or
 * data to be ensured and, more generally, to use and operate it in the
 * same conditions as regards security.
 *
 * The fact that you are presently reading this means that you have had
 * knowledge of the CeCILL-C license and that you accept its terms.
 */

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

#endif
