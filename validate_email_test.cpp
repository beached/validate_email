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
	BOOST_REQUIRE_MESSAGE( test_address( "test@example.com" ), "Basic test" );
	BOOST_REQUIRE_MESSAGE( test_address( u8"test@Bücher.ch" ), "Valid IDN domain" );
	BOOST_REQUIRE_MESSAGE( test_address( u8"\"@ test\"@Bücher.ch" ), "Quotes, @, and space in local plus IDN test" );
	BOOST_REQUIRE_MESSAGE( test_address( "test@[127.0.0.1]" ), "Bracketed ip addrress" );
	BOOST_REQUIRE_MESSAGE( test_address( "test@127.0.0.1" ), "ip addrress" );
	BOOST_REQUIRE_MESSAGE( test_address( "test@8.8.8.8" ), "ip addrress" );
	BOOST_REQUIRE_MESSAGE( test_address( "email@domain.com" ), "Valid email" );
	BOOST_REQUIRE_MESSAGE( test_address( "firstname.lastname@domain.com" ), "Email contains dot in the address field" );
	BOOST_REQUIRE_MESSAGE( test_address( "email@subdomain.domain.com" ), "Email contains dot with subdomain" );
	BOOST_REQUIRE_MESSAGE( test_address( "firstname+lastname@domain.com" ), "Plus sign is considered valid character" );
	BOOST_REQUIRE_MESSAGE( test_address( "email@123.123.123.123" ), "Domain is valid IP address" );
	BOOST_REQUIRE_MESSAGE( test_address( "email@[123.123.123.123]" ), "Square bracket around IP address is considered valid" );
	BOOST_REQUIRE_MESSAGE( test_address( "“email”@domain.com" ), "Quotes around email is considered valid" );
	BOOST_REQUIRE_MESSAGE( test_address( "1234567890@domain.com" ), "Digits in address are valid" );
	BOOST_REQUIRE_MESSAGE( test_address( "email@domain-one.com" ), "Dash in domain name is valid" );
	BOOST_REQUIRE_MESSAGE( test_address( "_______@domain.com" ), "Underscore in the address field is valid" );
	BOOST_REQUIRE_MESSAGE( test_address( "email@domain.co.jp" ), "Dot in Top Level Domain name also considered valid (use co.jp as example here)" );
	BOOST_REQUIRE_MESSAGE( test_address( "firstname-lastname@domain.com" ), "Dash in address field is valid" );
	std::cout << "\n" << std::endl;
}

BOOST_AUTO_TEST_CASE( bad_email_test_001 ) {
	std::cout << "Bad email addresses\n";
	BOOST_REQUIRE_MESSAGE( !test_address( "email@domain..com" ), "Double dot in domain" );
	BOOST_REQUIRE_MESSAGE( !test_address( "#@%^%#$@#$@#.com" ), "Garbage" );
	BOOST_REQUIRE_MESSAGE( !test_address( u8"test@افغانستا.icom.museum" ), "Non-Existant IDN Domain test" );
	BOOST_REQUIRE_MESSAGE( !test_address( "plainaddress" ), "Missing @ sign and domain" );
	BOOST_REQUIRE_MESSAGE( !test_address( "#@%^%#$@#$@#.com" ), "Garbage" );
	BOOST_REQUIRE_MESSAGE( !test_address( "@domain.com" ), "Missing username" );
	BOOST_REQUIRE_MESSAGE( !test_address( "Joe Smith <email@domain.com>" ), "Encoded html within email is invalid" );
	BOOST_REQUIRE_MESSAGE( !test_address( "email.domain.com" ), "Missing @" );
	BOOST_REQUIRE_MESSAGE( !test_address( "email@domain@domain.com" ), "Two @ sign no quote" );
	BOOST_REQUIRE_MESSAGE( !test_address( ".email@domain.com" ), "Leading dot in address is not allowed" );
	BOOST_REQUIRE_MESSAGE( !test_address( "email.@domain.com" ), "Trailing dot in address is not allowed" );
	BOOST_REQUIRE_MESSAGE( !test_address( "email..email@domain.com" ), "Multiple dots" );
//	BOOST_REQUIRE_MESSAGE( !test_address( "あいうえお@domain.com" ), "Unicode char as address" );
	BOOST_REQUIRE_MESSAGE( !test_address( "email@domain.com (Joe Smith)" ), "Text followed email is not allowed" );
	BOOST_REQUIRE_MESSAGE( !test_address( "email@domain" ), "Missing top level domain (.com/.net/.org/etc)" );
	BOOST_REQUIRE_MESSAGE( !test_address( "email@-domain.com" ), "Leading dash in front of domain is invalid" );
	BOOST_REQUIRE_MESSAGE( !test_address( "email@domain.web" ), ".web is not a valid top level domain" );
	BOOST_REQUIRE_MESSAGE( !test_address( "email@111.222.333.44444" ), "Invalid IP format" );
	BOOST_REQUIRE_MESSAGE( !test_address( "email@domain..com" ), "Multiple dot in the domain portion is invalid" );

	std::cout << "\n" << std::endl;
}

