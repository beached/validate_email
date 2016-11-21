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
#include <daw/puny_coder/puny_coder.h>

#include "validate_email.h"

struct address_tests_t: public daw::json::JsonLink<address_tests_t> {
	struct address_test_t: public daw::json::JsonLink<address_test_t> {
		std::string email_address;
		std::string comment;

		address_test_t( ):
				daw::json::JsonLink<address_test_t>{ },
				email_address{ },
				comment{ } { 
			
			set_links( );
		}

		address_test_t( address_test_t const & other ):
				daw::json::JsonLink<address_test_t>{ },
				email_address{ other.email_address },
				comment{ other.comment } { 
			
			set_links( );
		}

		address_test_t( address_test_t && other ):
				daw::json::JsonLink<address_test_t>{ },
				email_address{ std::move( other.email_address ) },
				comment{ std::move( other.comment ) } { 
			
			set_links( );
		}
		~address_test_t( ) = default;
		address_test_t & operator=( address_test_t const & ) = default;
		address_test_t & operator=( address_test_t && ) = default;

	private:
		void set_links( ) {
			link_string( "email_address", email_address );
			link_string( "comment", comment );
		}
	};	// address_test_t


	std::vector<address_test_t> tests;

	address_tests_t( ):
			daw::json::JsonLink<address_tests_t>{ },
			tests{ } {

		link_array( "tests", tests );
	}

	address_tests_t( address_tests_t const & other ):
			daw::json::JsonLink<address_tests_t>{ },
			tests{ other.tests } {
		
		link_array( "tests", tests );
	}

	address_tests_t( address_tests_t && other ):
			daw::json::JsonLink<address_tests_t>{ },
			tests{ std::move( other.tests ) } {
		
		link_array( "tests", tests );
	}

	~address_tests_t( ) = default;
	address_tests_t & operator=( address_tests_t const & ) = default;
	address_tests_t & operator=( address_tests_t && ) = default;
};	// address_tests_t

struct puny_tests_t: public daw::json::JsonLink<puny_tests_t> {
	struct puny_test_t: public daw::json::JsonLink<puny_test_t> {
		std::string in;
		std::string out;

		puny_test_t( ):
				daw::json::JsonLink<puny_test_t>{ },
				in{ },
				out{ } {

			set_links( );
		}

		puny_test_t( puny_test_t const & other ):
				daw::json::JsonLink<puny_test_t>{ },
				in{ other.in },
				out{ other.out } {

			set_links( );
		}

		puny_test_t( puny_test_t && other ):
				daw::json::JsonLink<puny_test_t>{ },
				in{ std::move( other.in ) },
				out{ std::move( other.out ) } {

			set_links( );
		}

		puny_test_t & operator=( puny_test_t const & ) = default;
		puny_test_t & operator=( puny_test_t && ) = default;
		~puny_test_t( ) = default;
	private:
		void set_links( ) {
			link_string( "in", in );
			link_string( "out", out );
		}
	};	// puny_test_t

	std::vector<puny_test_t> tests;

	puny_tests_t( ):
			daw::json::JsonLink<puny_tests_t>{ },
			tests{ } {

		link_array( "tests", tests );
	}

	puny_tests_t( puny_tests_t const & other ):
			daw::json::JsonLink<puny_tests_t>{ },
			tests{ other.tests } {

		link_array( "tests", tests );
	}
	
	puny_tests_t( puny_tests_t && other ):
			daw::json::JsonLink<puny_tests_t>{ },
			tests{ std::move( other.tests ) } {

		link_array( "tests", tests );
	}
		
	puny_tests_t & operator=( puny_tests_t const & ) = default;
	puny_tests_t & operator=( puny_tests_t && ) = default;
	~puny_tests_t( ) = default;
};	// puny_tests_t

bool test_address( boost::string_view address ) {
	std::cout << "Testing: " << address.data( );
	std::cout << " Puny: " << daw::get_local_part( address ) << "@" << daw::to_puny_code( daw::get_domain_part( address ) ) << std::endl;
	return daw::is_email_address( address );
}

BOOST_AUTO_TEST_CASE( good_email_test ) {
	std::cout << "\n\nGood email addresses\n";
	auto config_data = address_tests_t{ }.from_file( "../good_addresses.json" );
	for( auto const & address : config_data.tests ) {
		BOOST_REQUIRE_MESSAGE( test_address( address.email_address ), address.comment );
	}
	std::cout << std::endl;
}

BOOST_AUTO_TEST_CASE( bad_email_test ) {
	std::cout << "\nBad email addresses\n";
	auto config_data = address_tests_t{ }.from_file( "../bad_addresses.json" );
	for( auto const & address : config_data.tests ) {
		BOOST_REQUIRE_MESSAGE( !test_address( address.email_address ), address.comment );
	}
	std::cout << "\n" << std::endl;
}


