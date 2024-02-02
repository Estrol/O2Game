/*
 * MIT License
 *
 * Copyright (c) 2023 Estrol Mendex
 * See the LICENSE file in the root of this project for details.
 */

#pragma once
#include <deque>
#include <iostream>
#include <queue>

template <typename T, typename Container = std::deque<T>>
class iterable_queue : public std::queue<T, Container>
{
public:
    typedef typename Container::iterator       iterator;
    typedef typename Container::const_iterator const_iterator;

    iterator       begin() { return this->c.begin(); }
    iterator       end() { return this->c.end(); }
    const_iterator begin() const { return this->c.begin(); }
    const_iterator end() const { return this->c.end(); }
};