//
// Created by Colin McDonald on 3/14/26.
//

#ifndef PROJ_BIN_SEARCH_H
#define PROJ_BIN_SEARCH_H

template<class RandomIt>
RandomIt rotated_bin_search(RandomIt first, RandomIt last, int target) {
    if (first == last) {
        return last;
    }
    RandomIt mid = first + (last - first) / 2;
    if (*mid == target) {
        return mid;
    }
    if (*first <= *mid) {
        if (*first <= target && target < *mid ) {
            return rotated_bin_search(first, mid , target);
        }
        return rotated_bin_search(mid + 1, last, target);
    }
    if (*mid < target && target <= *(last - 1) ) {
        return rotated_bin_search(mid + 1, last, target);
    }
    return rotated_bin_search(first, mid, target);
}

#endif //PROJ_BIN_SEARCH_H