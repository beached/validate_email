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

#include "validate_email.h"
#include "punycode.h"

bool test_address( boost::string_ref address ) {
	std::cout << "Testing: " << address.data( );
	std::cout << " Puny: " << daw::get_local_part( address ) << "@" << daw::to_puny_code( daw::get_domain_part( address ) ) << std::endl;
	return daw::is_email_address( address );
}


BOOST_AUTO_TEST_CASE( good_email_test_001 ) {
	std::cout << "Good email addresses\n";
	BOOST_REQUIRE_MESSAGE( test_address( "test@example.com" ), "Good 001. Basic test" );
	BOOST_REQUIRE_MESSAGE( test_address( u8"test@Bücher.ch" ), "Good 002. Valid IDN domain" );
	BOOST_REQUIRE_MESSAGE( test_address( u8"\"@ test\"@Bücher.ch" ), "Good 003. Quotes, @, and space in local plus IDN test" );
	BOOST_REQUIRE_MESSAGE( test_address( "test@[127.0.0.1]" ), "Good 004. Bracketed ip addrress" );
	BOOST_REQUIRE_MESSAGE( test_address( "test@127.0.0.1" ), "Good 005. ip addrress" );
	BOOST_REQUIRE_MESSAGE( test_address( "test@8.8.8.8" ), "Good 006. ip addrress" );
}

BOOST_AUTO_TEST_CASE( bad_email_test_001 ) {
	std::cout << "Bad email addresses\n";
	BOOST_REQUIRE_MESSAGE( !test_address( "email@domain..com" ), "Bad 001. Double dot in domain" );
	BOOST_REQUIRE_MESSAGE( !test_address( "#@%^%#$@#$@#.com" ), "Bad 002. Garbage" );
	BOOST_REQUIRE_MESSAGE( !test_address( u8"test@افغانستا.icom.museum" ), "Bad 003. Non-Existant IDN Domain test" );
}

