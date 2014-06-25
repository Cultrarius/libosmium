#ifdef STAND_ALONE
# define BOOST_TEST_MODULE Main
#endif
#include <boost/test/unit_test.hpp>

#include <osmium/builder/builder_helper.hpp>
#include <osmium/geom/geojson.hpp>

#include "../basic/helper.hpp"

BOOST_AUTO_TEST_SUITE(GeoJSON_Geometry)

BOOST_AUTO_TEST_CASE(point) {
    osmium::geom::GeoJSONFactory factory;

    std::string json {factory.create_point(osmium::Location(3.2, 4.2))};
    BOOST_CHECK_EQUAL(std::string{"{\"type\":\"Point\",\"coordinates\":[3.2,4.2]}"}, json);
}

BOOST_AUTO_TEST_CASE(empty_point) {
    osmium::geom::GeoJSONFactory factory;

    BOOST_CHECK_THROW(factory.create_point(osmium::Location()), osmium::invalid_location);
}

BOOST_AUTO_TEST_CASE(linestring) {
    osmium::geom::GeoJSONFactory factory;

    osmium::memory::Buffer buffer(10000);
    auto& wnl = osmium::builder::build_way_node_list(buffer, {
        {1, {3.2, 4.2}},
        {3, {3.5, 4.7}},
        {4, {3.5, 4.7}},
        {2, {3.6, 4.9}}
    });

    {
        std::string json {factory.create_linestring(wnl)};
        BOOST_CHECK_EQUAL(std::string{"{\"type\":\"LineString\",\"coordinates\":[[3.2,4.2],[3.5,4.7],[3.6,4.9]]}"}, json);
    }

    {
        std::string json {factory.create_linestring(wnl, osmium::geom::use_nodes::unique, osmium::geom::direction::backward)};
        BOOST_CHECK_EQUAL(std::string{"{\"type\":\"LineString\",\"coordinates\":[[3.6,4.9],[3.5,4.7],[3.2,4.2]]}"}, json);
    }

    {
        std::string json {factory.create_linestring(wnl, osmium::geom::use_nodes::all)};
        BOOST_CHECK_EQUAL(std::string{"{\"type\":\"LineString\",\"coordinates\":[[3.2,4.2],[3.5,4.7],[3.5,4.7],[3.6,4.9]]}"}, json);
    }

    {
        std::string json {factory.create_linestring(wnl, osmium::geom::use_nodes::all, osmium::geom::direction::backward)};
        BOOST_CHECK_EQUAL(std::string{"{\"type\":\"LineString\",\"coordinates\":[[3.6,4.9],[3.5,4.7],[3.5,4.7],[3.2,4.2]]}"}, json);
    }
}

BOOST_AUTO_TEST_CASE(empty_linestring) {
    osmium::geom::GeoJSONFactory factory;

    osmium::memory::Buffer buffer(10000);
    auto& wnl = osmium::builder::build_way_node_list(buffer, {});

    BOOST_CHECK_THROW(factory.create_linestring(wnl), osmium::geometry_error);
    BOOST_CHECK_THROW(factory.create_linestring(wnl, osmium::geom::use_nodes::unique, osmium::geom::direction::backward), osmium::geometry_error);
    BOOST_CHECK_THROW(factory.create_linestring(wnl, osmium::geom::use_nodes::all), osmium::geometry_error);
    BOOST_CHECK_THROW(factory.create_linestring(wnl, osmium::geom::use_nodes::all, osmium::geom::direction::backward), osmium::geometry_error);
}

BOOST_AUTO_TEST_CASE(linestring_with_two_same_locations) {
    osmium::geom::GeoJSONFactory factory;

    osmium::memory::Buffer buffer(10000);
    auto& wnl = osmium::builder::build_way_node_list(buffer, {
        {1, {3.5, 4.7}},
        {2, {3.5, 4.7}}
    });

    BOOST_CHECK_THROW(factory.create_linestring(wnl), osmium::geometry_error);
    BOOST_CHECK_THROW(factory.create_linestring(wnl, osmium::geom::use_nodes::unique, osmium::geom::direction::backward), osmium::geometry_error);

    {
        std::string json {factory.create_linestring(wnl, osmium::geom::use_nodes::all)};
        BOOST_CHECK_EQUAL(std::string{"{\"type\":\"LineString\",\"coordinates\":[[3.5,4.7],[3.5,4.7]]}"}, json);
    }

    {
        std::string json {factory.create_linestring(wnl, osmium::geom::use_nodes::all, osmium::geom::direction::backward)};
        BOOST_CHECK_EQUAL(std::string{"{\"type\":\"LineString\",\"coordinates\":[[3.5,4.7],[3.5,4.7]]}"}, json);
    }
}

BOOST_AUTO_TEST_CASE(linestring_with_undefined_location) {
    osmium::geom::GeoJSONFactory factory;

    osmium::memory::Buffer buffer(10000);
    auto& wnl = osmium::builder::build_way_node_list(buffer, {
        {1, {3.5, 4.7}},
        {2, osmium::Location()}
    });

    BOOST_CHECK_THROW(factory.create_linestring(wnl), osmium::invalid_location);
}

BOOST_AUTO_TEST_CASE(area_1outer_0inner) {
    osmium::geom::GeoJSONFactory factory;

    osmium::memory::Buffer buffer(10000);
    osmium::Area& area = buffer_add_area(buffer,
        "foo",
        {},
        {
            { true, {
                {1, {3.2, 4.2}},
                {2, {3.5, 4.7}},
                {3, {3.6, 4.9}},
                {1, {3.2, 4.2}}
            }}
        });

    {
        std::string json {factory.create_multipolygon(area)};
        BOOST_CHECK_EQUAL(std::string{"{\"type\":\"MultiPolygon\",\"coordinates\":[[[[3.2,4.2],[3.5,4.7],[3.6,4.9],[3.2,4.2]]]]}"}, json);
    }
}

BOOST_AUTO_TEST_CASE(area_1outer_1inner) {
    osmium::geom::GeoJSONFactory factory;

    osmium::memory::Buffer buffer(10000);
    osmium::Area& area = buffer_add_area(buffer,
        "foo",
        {},
        {
            { true, {
                {1, {0.1, 0.1}},
                {2, {9.1, 0.1}},
                {3, {9.1, 9.1}},
                {4, {0.1, 9.1}},
                {1, {0.1, 0.1}}
            }},
            { false, {
                {5, {1.0, 1.0}},
                {6, {8.0, 1.0}},
                {7, {8.0, 8.0}},
                {8, {1.0, 8.0}},
                {5, {1.0, 1.0}}
            }}
        });

    {
        std::string json {factory.create_multipolygon(area)};
        BOOST_CHECK_EQUAL(std::string{"{\"type\":\"MultiPolygon\",\"coordinates\":[[[[0.1,0.1],[9.1,0.1],[9.1,9.1],[0.1,9.1],[0.1,0.1]],[[1,1],[8,1],[8,8],[1,8],[1,1]]]]}"}, json);
    }
}

BOOST_AUTO_TEST_CASE(area_2outer_2inner) {
    osmium::geom::GeoJSONFactory factory;

    osmium::memory::Buffer buffer(10000);
    osmium::Area& area = buffer_add_area(buffer,
        "foo",
        {},
        {
            { true, {
                {1, {0.1, 0.1}},
                {2, {9.1, 0.1}},
                {3, {9.1, 9.1}},
                {4, {0.1, 9.1}},
                {1, {0.1, 0.1}}
            }},
            { false, {
                {5, {1.0, 1.0}},
                {6, {4.0, 1.0}},
                {7, {4.0, 4.0}},
                {8, {1.0, 4.0}},
                {5, {1.0, 1.0}}
            }},
            { false, {
                {10, {5.0, 5.0}},
                {11, {5.0, 7.0}},
                {12, {7.0, 7.0}},
                {10, {5.0, 5.0}}
            }},
            { true, {
                {100, {10.0, 10.0}},
                {101, {11.0, 10.0}},
                {102, {11.0, 11.0}},
                {103, {10.0, 11.0}},
                {100, {10.0, 10.0}}
            }}
        });

    {
        std::string json {factory.create_multipolygon(area)};
        BOOST_CHECK_EQUAL(std::string{"{\"type\":\"MultiPolygon\",\"coordinates\":[[[[0.1,0.1],[9.1,0.1],[9.1,9.1],[0.1,9.1],[0.1,0.1]],[[1,1],[4,1],[4,4],[1,4],[1,1]],[[5,5],[5,7],[7,7],[5,5]]],[[[10,10],[11,10],[11,11],[10,11],[10,10]]]]}"}, json);
    }
}

BOOST_AUTO_TEST_SUITE_END()
