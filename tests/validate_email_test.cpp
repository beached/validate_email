// The MIT License (MIT)
//
// Copyright (c) 2016 Darrell Wright
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files( the "Software" ), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#define BOOST_TEST_MODULE validate_email

#include <boost/test/unit_test.hpp>
#include <iostream>

#include <daw/daw_string_view.h>
#include <daw/json/daw_json_link.h>
#include <daw/json/daw_json_link_file.h>
#include <daw/puny_coder/puny_coder.h>

#include "validate_email.h"

struct address_tests_t: public daw::json::daw_json_link<address_tests_t> {
	struct address_test_t: public daw::json::daw_json_link<address_test_t> {
		std::string email_address;
		std::string comment;

		static void json_link_map( ) {
			link_json_string( "email_address", email_address );
			link_json_string( "comment", comment );
		}
	};	// address_test_t

	std::vector<address_test_t> tests;

	static void json_link_map( ) {
		link_json_object_array( "tests", tests );
	}
};	// address_tests_t

struct puny_tests_t: public daw::json::daw_json_link<puny_tests_t> {
	struct puny_test_t: public daw::json::daw_json_link<puny_test_t> {
		std::string in;
		std::string out;

		static void json_link_map( ) {
			link_json_string( "in", in );
			link_json_string( "out", out );
		}
	};	// puny_test_t

	std::vector<puny_test_t> tests;

	static void json_link_map( ) {
		link_json_object_array( "tests", tests );
	}
};	// puny_tests_t

bool test_address( daw::string_view address ) {
	std::cout << "Testing: " << address.data( );
	std::cout << " Puny: " << daw::get_local_part( address ) << "@" << daw::to_puny_code( daw::get_domain_part( address ) ) << std::endl;
	auto result = daw::is_email_address( address );
	return result;
}

BOOST_AUTO_TEST_CASE( good_email_test ) {
	std::cout << "\n\nGood email addresses\n";
	auto config_data = daw::json::from_file<address_tests_t>( "../good_addresses.json" );
	for( auto const & address : config_data.tests ) {
		BOOST_REQUIRE_MESSAGE( test_address( address.email_address ), address.comment );
	}
	std::cout << std::endl;
}

BOOST_AUTO_TEST_CASE( bad_email_test ) {
	std::cout << "\nBad email addresses\n";
	auto config_data = daw::json::from_file<address_tests_t>( "../bad_addresses.json" );
	for( auto const & address : config_data.tests ) {
		BOOST_REQUIRE_MESSAGE( !test_address( address.email_address ), address.comment );
	}
	std::cout << "\n" << std::endl;
}


