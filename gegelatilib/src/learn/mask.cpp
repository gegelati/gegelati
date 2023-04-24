#include <utility>

#include "../../include/learn/mask.h"

void Mask::updateMask(uint64_t mask_index)
{

    // Make sure all the weights are above the threshold to avoid null values
    for(auto & m : this->all_masks.at(mask_index))
        for(auto & p : m)
            p = p < THRESHOLD ? THRESHOLD : p;
}

std::vector<int> Mask::getIdx(Mutator::RNG rng, uint64_t mask_index) // utiliser un rng en parametre pour para
{
    int x = 0, y = 0, temp_idx = 0;
    double n = 0.0, temp_value = 0.0;

    auto mask = this->all_masks.at(mask_index);

    // Choosing a column
    std::vector<double> columns;
    for(auto & i : mask)
        columns.push_back(std::accumulate(i.begin(), i.end(), 0.0));    // Sum of the column content

    // x = random index of a column according to :
    //      - n = random double between 0 and std::accumulate(columns.begin(), columns.end(), 0)

    n = rng.getDouble(0.0, std::accumulate(columns.begin(), columns.end(), 0.0));

    //      - x = last index of columns where the sum of previous index values is lower than n
    do {
        temp_value += columns.at(temp_idx++);
    } while(temp_value < n);

    x = temp_idx;

    // Choosing a line
    std::vector<double> lines;
    for(int i=0 ; i<mask.at(0).size() ; i++)
        lines.push_back(std::accumulate(mask.at(x).begin(), mask.at(x).end(), 0.0));

    for(auto & i : mask)
    {
        double count = 0.0;
        for (double j : i)
            count += j;
        lines.push_back(count);
    }

    // y = random index of a line according to :
    //      - n = random double between 0 and std::accumulate(lines.begin(), lines.end(), 0)

    n = rng.getDouble(0.0, std::accumulate(lines.begin(), lines.end(), 0.0));

    //      - y = last index of columns where the sum of previous index values is lower than n
    temp_idx = 0;
    temp_value = 0.0;
    do {
        temp_value += lines.at(temp_idx++);
    } while(temp_value < n);

    y = temp_idx;

    std::vector<int> res{x, y};
    return res;
}

void Mask::init(std::vector<mask_t> &masks)
{
    this->all_masks = masks;
}
std::vector<int> Mask::getSize(uint64_t mask_index)
{
    return std::vector<int>{static_cast<int>(this->all_masks.at(mask_index).size()), static_cast<int>(this->all_masks.at(mask_index).begin()->size())};
}
