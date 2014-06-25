#ifndef OSMIUM_DIFF_VISITOR_HPP
#define OSMIUM_DIFF_VISITOR_HPP

/*

This file is part of Osmium (http://osmcode.org/libosmium).

Copyright 2013,2014 Jochen Topf <jochen@topf.org> and others (see README).

Boost Software License - Version 1.0 - August 17th, 2003

Permission is hereby granted, free of charge, to any person or organization
obtaining a copy of the software and accompanying documentation covered by
this license (the "Software") to use, reproduce, display, distribute,
execute, and transmit the Software, and to prepare derivative works of the
Software, and to permit third-parties to whom the Software is furnished to
do so, all subject to the following:

The copyright notices in the Software and this entire statement, including
the above license grant, this restriction and the following disclaimer,
must be included in all copies of the Software, in whole or in part, and
all derivative works of the Software, unless such copies or derivative
works are solely in the form of machine-executable object code generated by
a source language processor.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.

*/

#include <stdexcept>

#include <osmium/diff_iterator.hpp>
#include <osmium/io/input_iterator.hpp>
#include <osmium/memory/buffer.hpp>
#include <osmium/osm/diff_object.hpp>
#include <osmium/osm/item_type.hpp>

namespace osmium {

    namespace detail {

        template <class THandler>
        inline void apply_diff_iterator_recurse(const osmium::DiffObject& diff, THandler& handler) {
            switch (diff.type()) {
                case osmium::item_type::node:
                    handler.node(static_cast<const osmium::DiffNode&>(diff));
                    break;
                case osmium::item_type::way:
                    handler.way(static_cast<const osmium::DiffWay&>(diff));
                    break;
                case osmium::item_type::relation:
                    handler.relation(static_cast<const osmium::DiffRelation&>(diff));
                    break;
                default:
                    throw std::runtime_error("unknown type");
            }
        }

        template <class THandler, class ...TRest>
        inline void apply_diff_iterator_recurse(const osmium::DiffObject& diff, THandler& handler, TRest&... more) {
            apply_diff_iterator_recurse(diff, handler);
            apply_diff_iterator_recurse(diff, more...);
        }

    } // namespace detail

    template <class TIterator, class ...THandlers>
    inline void apply_diff(TIterator it, TIterator end, THandlers&... handlers) {
        typedef osmium::DiffIterator<TIterator> diff_iterator;

        diff_iterator dit(it, end);
        diff_iterator dend(end, end);

        for (; dit != dend; ++dit) {
            detail::apply_diff_iterator_recurse(*dit, handlers...);
        }
    }

    class OSMObject;

    template <class TSource, class ...THandlers>
    inline void apply_diff(TSource& source, THandlers&... handlers) {
        apply_diff(osmium::io::InputIterator<TSource, osmium::OSMObject> {source},
                   osmium::io::InputIterator<TSource, osmium::OSMObject> {},
                   handlers...);
    }

    template <class ...THandlers>
    inline void apply_diff(osmium::memory::Buffer& buffer, THandlers&... handlers) {
        apply_diff(buffer.begin(), buffer.end(), handlers...);
    }

    template <class ...THandlers>
    inline void apply_diff(const osmium::memory::Buffer& buffer, THandlers&... handlers) {
        apply_diff(buffer.cbegin(), buffer.cend(), handlers...);
    }

} // namespace osmium

#endif // OSMIUM_DIFF_VISITOR_HPP