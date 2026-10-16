/*******************************************************************************
 * Copyright (C) 2026 Julien Bigot <julien@julien-bigot.fr>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * * Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * * Neither the name of CEA nor the names of its contributors may be used to
 *   endorse or promote products derived from this software without specific
 *   prior written permission.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 ******************************************************************************/

#include <memory>
#include <string>
#include <string_view>

#include <gtest/gtest.h>

#include <pdi/array_datatype.h>
#include <pdi/error.h>
#include <pdi/expression.h>
#include <pdi/ref_any.h>
#include <pdi/scalar_datatype.h>

#include "mocks/context_mock.h"
#include "mocks/data_descriptor_mock.h"

using PDI::Array_datatype;
using PDI::Error;
using PDI::Expression;
using PDI::RAW_STRING;
using PDI::Ref;
using PDI::Ref_r;
using PDI::Scalar_datatype;
using PDI::Scalar_kind;
using std::string;
using std::string_view;

namespace {

/** Makes a data descriptor of a context return a char array holding exactly the characters of a string, without any terminating null
 */
void expose_string(MockContext& ctx, MockDataDescriptor& desc, const char* name, string& content)
{
	EXPECT_CALL(ctx, desc(testing::Matcher<const char*>(testing::StrEq(name)))).WillRepeatedly(testing::ReturnRef(desc));
	EXPECT_CALL(desc, ref()).WillRepeatedly(testing::Invoke([&content] {
		return Ref(
			content.data(),
			[](void*) {},
			Array_datatype::make(Scalar_datatype::make(Scalar_kind::UNSIGNED, sizeof(char)), content.size()),
			true,
			true
		);
	}));
}

} // namespace

struct ExpressionStringTest: public testing::Test {
	MockContext ctx;
};

TEST_F(ExpressionStringTest, raw_string_is_not_parsed)
{
	EXPECT_EQ(Expression(RAW_STRING, "$a ${b} $(1+2) \\ 3").to_string(ctx), "$a ${b} $(1+2) \\ 3");
	EXPECT_EQ(Expression(RAW_STRING, string("$a")).to_string(ctx), "$a");
	EXPECT_EQ(Expression(RAW_STRING, string_view("$a")).to_string(ctx), "$a");
}

TEST_F(ExpressionStringTest, raw_string_is_not_a_number)
{
	EXPECT_EQ(Expression(RAW_STRING, "1 + 2").to_string(ctx), "1 + 2");
	EXPECT_THROW(Expression(RAW_STRING, "1 + 2").to_long(ctx), Error);
}

TEST_F(ExpressionStringTest, empty_raw_string_is_not_empty)
{
	EXPECT_TRUE(Expression(RAW_STRING, ""));
	EXPECT_EQ(Expression(RAW_STRING, "").to_string(ctx), "");
}

TEST_F(ExpressionStringTest, string_view_is_parsed)
{
	EXPECT_EQ(Expression(string_view("2 + 3")).to_long(ctx), 5);
	EXPECT_EQ(Expression(string_view("a string")).to_string(ctx), "a string");
}

TEST_F(ExpressionStringTest, concatenation)
{
	Expression result = Expression("foo") + Expression("bar");
	EXPECT_EQ(result.to_string(ctx), "foobar");
}

TEST_F(ExpressionStringTest, concatenation_to_ref)
{
	Ref_r ref{(Expression(RAW_STRING, "foo") + Expression(RAW_STRING, "bar")).to_ref(ctx)};
	auto array_type = std::dynamic_pointer_cast<const Array_datatype>(ref.type());
	ASSERT_TRUE(array_type);
	EXPECT_EQ(array_type->size(), 7);
	EXPECT_STREQ(static_cast<const char*>(ref.get()), "foobar");
}

TEST_F(ExpressionStringTest, raw_strings_concatenation_is_not_parsed)
{
	EXPECT_EQ((Expression(RAW_STRING, "$a") + Expression(RAW_STRING, "${b}")).to_string(ctx), "$a${b}");
}

TEST_F(ExpressionStringTest, chained_concatenation)
{
	EXPECT_EQ((Expression("a") + Expression("b") + Expression("c")).to_string(ctx), "abc");
}

TEST_F(ExpressionStringTest, concatenation_with_empty_string)
{
	EXPECT_EQ((Expression(RAW_STRING, "") + Expression("bar")).to_string(ctx), "bar");
	EXPECT_EQ((Expression("foo") + Expression(RAW_STRING, "")).to_string(ctx), "foo");
}

TEST_F(ExpressionStringTest, concatenation_of_strings_with_references)
{
	string first = "hello";
	string second = "world";
	MockDataDescriptor first_desc;
	MockDataDescriptor second_desc;
	expose_string(ctx, first_desc, "first", first);
	expose_string(ctx, second_desc, "second", second);

	EXPECT_EQ((Expression("<${first}") + Expression("${second}>")).to_string(ctx), "<helloworld>");
	EXPECT_EQ((Expression("${first}_") + Expression("_${second}")).to_string(ctx), "hello__world");
	EXPECT_EQ((Expression("${first}${first}") + Expression("${second}${second}")).to_string(ctx), "hellohelloworldworld");
	EXPECT_EQ((Expression("${first}${first}") + Expression("x")).to_string(ctx), "hellohellox");
}

TEST_F(ExpressionStringTest, concatenation_is_evaluated_late)
{
	string first = "a";
	string second = "b";
	MockDataDescriptor first_desc;
	MockDataDescriptor second_desc;
	expose_string(ctx, first_desc, "first", first);
	expose_string(ctx, second_desc, "second", second);

	Expression result = Expression("${first}.") + Expression(".${second}");
	EXPECT_EQ(result.to_string(ctx), "a..b");
	second[0] = 'c';
	EXPECT_EQ(result.to_string(ctx), "a..c");
}

TEST_F(ExpressionStringTest, plus_of_references_is_not_a_concatenation)
{
	string first = "hello";
	string second = "world";
	MockDataDescriptor first_desc;
	MockDataDescriptor second_desc;
	expose_string(ctx, first_desc, "first", first);
	expose_string(ctx, second_desc, "second", second);

	EXPECT_THROW(Expression("${first} + ${second}").to_string(ctx), Error);
	EXPECT_THROW((Expression("${first}") + Expression("${second}")).to_string(ctx), Error);
	EXPECT_THROW((Expression("${first}") + Expression("foo")).to_string(ctx), Error);
}

TEST_F(ExpressionStringTest, concatenation_with_a_number_is_an_error)
{
	EXPECT_THROW((Expression("foo") + Expression("1")).to_string(ctx), Error);
	EXPECT_THROW((Expression("1") + Expression("foo")).to_string(ctx), Error);
}

TEST_F(ExpressionStringTest, only_the_plus_operator_applies_to_strings)
{
	EXPECT_THROW((Expression("foo") - Expression("bar")).to_string(ctx), Error);
	EXPECT_THROW((Expression("foo") * Expression("bar")).to_string(ctx), Error);
	EXPECT_THROW((Expression("foo") / Expression("bar")).to_string(ctx), Error);
	EXPECT_THROW((Expression("foo") % Expression("bar")).to_string(ctx), Error);
}

TEST_F(ExpressionStringTest, concatenation_operands_are_left_untouched)
{
	Expression first{"foo"};
	Expression second{"bar"};
	Expression result = first + second;
	result += first;
	EXPECT_EQ(first.to_string(ctx), "foo");
	EXPECT_EQ(second.to_string(ctx), "bar");
	EXPECT_EQ(result.to_string(ctx), "foobarfoo");
}

TEST_F(ExpressionStringTest, self_concatenation)
{
	Expression a{"ab"};
	a += a;
	EXPECT_EQ(a.to_string(ctx), "abab");
}

TEST_F(ExpressionStringTest, concatenation_with_a_c_string)
{
	EXPECT_EQ((Expression(RAW_STRING, "foo") + "bar").to_string(ctx), "foobar");
}

TEST_F(ExpressionStringTest, plus_assign)
{
	Expression result{"foo"};
	result += Expression("bar");
	EXPECT_EQ(result.to_string(ctx), "foobar");
}

struct ExpressionAssignOperatorTest: public testing::Test {
	MockContext ctx;
};

TEST_F(ExpressionAssignOperatorTest, plus_assign)
{
	Expression a{"7"};
	a += Expression("3");
	EXPECT_EQ(a.to_long(ctx), (Expression("7") + Expression("3")).to_long(ctx));
	EXPECT_EQ(a.to_long(ctx), 10);
}

TEST_F(ExpressionAssignOperatorTest, minus_assign)
{
	Expression a{"7"};
	a -= Expression("3");
	EXPECT_EQ(a.to_long(ctx), (Expression("7") - Expression("3")).to_long(ctx));
	EXPECT_EQ(a.to_long(ctx), 4);
}

TEST_F(ExpressionAssignOperatorTest, mult_assign)
{
	Expression a{"7"};
	a *= Expression("3");
	EXPECT_EQ(a.to_long(ctx), (Expression("7") * Expression("3")).to_long(ctx));
	EXPECT_EQ(a.to_long(ctx), 21);
}

TEST_F(ExpressionAssignOperatorTest, div_assign)
{
	Expression a{"7"};
	a /= Expression("2");
	EXPECT_EQ(a.to_long(ctx), (Expression("7") / Expression("2")).to_long(ctx));
	EXPECT_EQ(a.to_long(ctx), 3);
}

TEST_F(ExpressionAssignOperatorTest, mod_assign)
{
	Expression a{"7"};
	a %= Expression("4");
	EXPECT_EQ(a.to_long(ctx), (Expression("7") % Expression("4")).to_long(ctx));
	EXPECT_EQ(a.to_long(ctx), 3);
}

TEST_F(ExpressionAssignOperatorTest, assign_returns_the_expression)
{
	Expression a{"1"};
	Expression& result = (a += Expression("1"));
	EXPECT_EQ(&result, &a);
	EXPECT_EQ((a += Expression("1") *= Expression("2")).to_long(ctx), 4);
}

TEST_F(ExpressionAssignOperatorTest, chained_assign)
{
	Expression a{"1"};
	a += Expression("2");
	a *= Expression("3");
	a -= Expression("4");
	EXPECT_EQ(a.to_long(ctx), 5);
}

TEST_F(ExpressionAssignOperatorTest, self_assign)
{
	Expression a{"3"};
	a += a;
	EXPECT_EQ(a.to_long(ctx), 6);
	a *= a;
	EXPECT_EQ(a.to_long(ctx), 36);
}

TEST_F(ExpressionAssignOperatorTest, operators_with_a_conversion)
{
	EXPECT_EQ((Expression("3") + 4L).to_long(ctx), 7);
	EXPECT_EQ((Expression("3") * 2.5).to_double(ctx), 7.5);
}

TEST_F(ExpressionAssignOperatorTest, assign_with_a_conversion)
{
	Expression a{"3"};
	a += 4L;
	a *= 2.;
	a -= "1";
	EXPECT_EQ(a.to_long(ctx), 13);
}

TEST_F(ExpressionAssignOperatorTest, assign_on_an_empty_expression_is_an_error)
{
	Expression empty;
	EXPECT_THROW(empty += Expression("1"), Error);
	EXPECT_THROW(empty -= Expression("1"), Error);
	EXPECT_THROW(empty *= Expression("1"), Error);
	EXPECT_THROW(empty /= Expression("1"), Error);
	EXPECT_THROW(empty %= Expression("1"), Error);
	EXPECT_FALSE(empty);
}

TEST_F(ExpressionAssignOperatorTest, assign_of_an_empty_expression_is_an_error)
{
	Expression a{"1"};
	EXPECT_THROW(a += Expression(), Error);
	EXPECT_EQ(a.to_long(ctx), 1);
}
