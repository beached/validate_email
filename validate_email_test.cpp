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

#include <daw/json/daw_json_link.h>
#include <daw/daw_memory_mapped_file.h>

#include "validate_email.h"
#include "punycode.h"

struct address_test_t: public daw::json::JsonLink<address_test_t> {
	std::string email_address;
	std::string comment;

	address_test_t( ):
			daw::json::JsonLink<address_test_t>{ },
			email_address{ },
			comment{ } { 
		
		link_string( "email_address", email_address );
		link_string( "comment", comment );
	}
};	// address_test_t

struct config_file_t: public daw::json::JsonLink<config_file_t> {
	std::vector<address_test_t> tests;

	config_file_t( ):
			daw::json::JsonLink<config_file_t>{ },
			tests{ } {

		link_array( "tests", tests );
	}
};	// config_file_t

template<typename T>
auto from_file( boost::string_ref filename ) {
	daw::filesystem::MemoryMappedFile<char> test_data( filename );

	T result;

	result.decode( test_data.begin( ), test_data.end( ) );

	return result;
}

bool test_address( boost::string_ref address ) {
	std::cout << "Testing: " << address.data( );
	std::cout << " Puny: " << daw::get_local_part( address ) << "@" << daw::to_puny_code( daw::get_domain_part( address ) ) << std::endl;
	return daw::is_email_address( address );
}


BOOST_AUTO_TEST_CASE( good_email_test_001 ) {
	std::cout << "Good email addresses\n";
	for( auto const & address : from_file<config_file_t>( "../good_addresses.json" ).tests ) {
		BOOST_REQUIRE_MESSAGE( test_address( address.email_address ), address.comment );
	}
	std::cout << "\n" << std::endl;
}

BOOST_AUTO_TEST_CASE( bad_email_test_001 ) {
	std::cout << "Bad email addresses\n";
	for( auto const & address : from_file<config_file_t>( "../bad_addresses.json" ).tests ) {
		BOOST_REQUIRE_MESSAGE( !test_address( address.email_address ), address.comment );
	}
	std::cout << "\n" << std::endl;
}

