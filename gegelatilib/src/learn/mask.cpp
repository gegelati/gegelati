#include "../../include/learn/mask.h"

void Mask::updateMask()
{

}

std::vector<int> Mask::getIdx()
{
    int x = 0, y = 0, temp_idx = 0;
    double n = 0.0, temp_value = 0.0;

    // Choosing a column
    std::vector<double> columns;
    for(auto & i : this->_mask)
        columns.push_back(std::accumulate(i.begin(), i.end(), 0.0));    // Sum of the column content

    // x = random index of a column according to :
    //      - n = random double between 0 and std::accumulate(columns.begin(), columns.end(), 0)
    //
    //      - x = last index of columns where the sum of previous index values is lower than n

    do {
        temp_value += columns.at(temp_idx++);
    } while(temp_value < n);

    x = temp_idx;

    // Choosing a line
    std::vector<double> lines;
    for(int i=0 ; i<this->_mask.at(0).size() ; i++)
        lines.push_back(std::accumulate(this->_mask.at(x).begin(), this->_mask.at(x).end(), 0.0));

    for(auto & i : this->_mask)
    {
        double count = 0.0;
        for (double j : i)
            count += j;
        lines.push_back(count);
    }

    // y = random index of a line according to :
    //      - n = random double between 0 and std::accumulate(lines.begin(), lines.end(), 0)
    //
    //      - y = last index of columns where the sum of previous index values is lower than n

    temp_idx = 0;
    temp_value = 0.0;
    do {
        temp_value += lines.at(temp_idx++);
    } while(temp_value < n);

    y = temp_idx;

    std::vector<int> res(2);
    res.push_back(x);
    res.push_back(y);
    return res;
}

void Mask::init()
{
    // Initialise a Mutator attribute with a given seed ?
}
