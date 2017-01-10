#include "util.h"
#include "toml2.h"

static toml2_t
check_init(const char *str)
{
	toml2_t doc;
	toml2_init(&doc);
	ck_assert_int_eq(0, toml2_parse(&doc, str, strlen(str)));
	return doc;
}

static void
check_err(toml2_errcode_t err, const char *str)
{
	toml2_t doc;
	toml2_init(&doc);
	ck_assert_int_eq(err, toml2_parse(&doc, str, strlen(str)));
	toml2_free(&doc);
}

START_TEST(init_free)
{
	toml2_t doc;
	toml2_init(&doc);
	toml2_free(&doc);
}
END_TEST

START_TEST(basic_table)
{
	toml2_t doc = check_init("[foo]\nbar = 42");
	ck_assert_int_eq(42, toml2_int(toml2_get_path(&doc, "foo.bar")));
	ck_assert_int_eq(1, toml2_len(&doc));
	ck_assert_int_eq(1, toml2_len(toml2_get(&doc, "foo")));
	ck_assert_str_eq("foo", toml2_name(toml2_index(&doc, 0)));
	ck_assert_str_eq("bar", toml2_name(toml2_index(toml2_index(&doc, 0), 0)));
	toml2_free(&doc);
}
END_TEST

START_TEST(root_value)
{
	toml2_t doc = check_init("int=1\n");
	ck_assert_int_eq(1, toml2_int(toml2_get(&doc, "int")));
	ck_assert_int_eq(1, toml2_len(&doc));
	toml2_free(&doc);
}
END_TEST

START_TEST(newlines)
{
	toml2_t doc = check_init("\n\n [foo]\nbar=3\n\n\n");
	ck_assert_int_eq(3, toml2_int(toml2_get_path(&doc, "foo.bar")));
	toml2_free(&doc);
}
END_TEST

START_TEST(comments)
{
	toml2_t doc = check_init("#hurr\n[foo]#foo\n#bar=3\nbar=4#ok");
	ck_assert_int_eq(4, toml2_int(toml2_get_path(&doc, "foo.bar")));
	toml2_free(&doc);
}
END_TEST

START_TEST(two_tables)
{
	toml2_t doc = check_init("[foo]\nfoo=2\n[bar]\nfoo=4");
	ck_assert_int_eq(2, toml2_int(toml2_get_path(&doc, "foo.foo")));
	ck_assert_int_eq(4, toml2_int(toml2_get_path(&doc, "bar.foo")));
	toml2_free(&doc);
}
END_TEST

START_TEST(table_ary)
{
	toml2_t doc = check_init("[[foo]]\nbar=10\n[[foo]]\nbar=20");
	ck_assert_int_eq(2, toml2_len(toml2_get_path(&doc, "foo")));
	ck_assert_int_eq(10, toml2_int(toml2_get(toml2_index(toml2_get(&doc, "foo"), 0), "bar")));
	ck_assert_int_eq(20, toml2_int(toml2_get(toml2_index(toml2_get(&doc, "foo"), 1), "bar")));
	toml2_free(&doc);
}
END_TEST

START_TEST(table_with_ary)
{
	toml2_t doc = check_init("[foo]\nbar=10\n[[foo.baz]]\nbar=20\n[[foo.baz]]\nbar=30");
	ck_assert_int_eq(10, toml2_int(toml2_get_path(&doc, "foo.bar")));
	ck_assert_int_eq(2, toml2_len(toml2_get_path(&doc, "foo.baz")));
	ck_assert_int_eq(20, toml2_int(toml2_get_path(&doc, "foo.baz.0.bar")));
	ck_assert_int_eq(30, toml2_int(toml2_get_path(&doc, "foo.baz.1.bar")));
	toml2_free(&doc);
}
END_TEST

START_TEST(inline_ary)
{
	toml2_t doc = check_init("x = [1, 2, 3]");
	ck_assert_int_eq(TOML2_LIST, toml2_type(toml2_get(&doc, "x")));
	ck_assert_int_eq(3, toml2_len(toml2_get(&doc, "x")));
	ck_assert_int_eq(1, toml2_int(toml2_get_path(&doc, "x.0")));
	ck_assert_int_eq(2, toml2_int(toml2_get_path(&doc, "x.1")));
	ck_assert_int_eq(3, toml2_int(toml2_get_path(&doc, "x.2")));
	toml2_free(&doc);
}
END_TEST

START_TEST(inline_obj)
{
	toml2_t doc = check_init("x = {'a':42, 'b':24}");
	ck_assert_int_eq(1, toml2_len(&doc));
	ck_assert_int_eq(2, toml2_len(toml2_get(&doc, "x")));
	ck_assert_int_eq(TOML2_TABLE, toml2_type(toml2_get(&doc, "x")));
	ck_assert_int_eq(TOML2_INT, toml2_type(toml2_get_path(&doc, "x.a")));
	ck_assert_int_eq(42, toml2_int(toml2_get_path(&doc, "x.a")));
	ck_assert_int_eq(TOML2_INT, toml2_type(toml2_get_path(&doc, "x.b")));
	ck_assert_int_eq(24, toml2_int(toml2_get_path(&doc, "x.b")));
	toml2_free(&doc);
}
END_TEST

START_TEST(empty_inline_ary)
{
	toml2_t doc = check_init("x = []");
	ck_assert_int_eq(1, toml2_len(&doc));
	ck_assert_int_eq(TOML2_LIST, toml2_type(toml2_get(&doc, "x")));
	ck_assert_int_eq(0, toml2_len(toml2_get(&doc, "x")));
	toml2_free(&doc);
}
END_TEST

START_TEST(empty_inline_obj)
{
	toml2_t doc = check_init("x = {}");
	ck_assert_int_eq(1, toml2_len(&doc));
	ck_assert_int_eq(TOML2_TABLE, toml2_type(toml2_get(&doc, "x")));
	ck_assert_int_eq(0, toml2_len(toml2_get(&doc, "x")));
	toml2_free(&doc);
}
END_TEST

START_TEST(inline_ary_obj)
{
	toml2_t doc = check_init("x = [ { 'y' : 4 } ]");
	ck_assert_int_eq(1, toml2_len(&doc));
	ck_assert_int_eq(TOML2_LIST, toml2_type(toml2_get(&doc, "x")));
	ck_assert_int_eq(TOML2_TABLE, toml2_type(toml2_get_path(&doc, "x.0")));
	ck_assert_int_eq(TOML2_INT, toml2_type(toml2_get_path(&doc, "x.0.y")));
	ck_assert_int_eq(4, toml2_int(toml2_get_path(&doc, "x.0.y")));
	toml2_free(&doc);
}
END_TEST

START_TEST(inline_obj_ary)
{
	toml2_t doc = check_init("x = { 'y' : [4] }");
	ck_assert_int_eq(1, toml2_len(&doc));
	ck_assert_int_eq(TOML2_TABLE, toml2_type(toml2_get(&doc, "x")));
	ck_assert_int_eq(TOML2_LIST, toml2_type(toml2_get_path(&doc, "x.y")));
	ck_assert_int_eq(TOML2_INT, toml2_type(toml2_get_path(&doc, "x.y.0")));
	ck_assert_int_eq(4, toml2_int(toml2_get_path(&doc, "x.y.0")));
	toml2_free(&doc);
}
END_TEST

START_TEST(err_mixed_inline_list)
{
	check_err(TOML2_MIXED_LIST, "x = [1, '2']");
}
END_TEST

Suite*
suite_grammar()
{
	tcase_t tests[] = {
		{ "init_free",             &init_free             },
		{ "basic_table",           &basic_table           },
		{ "root_value",            &root_value            },
		{ "newlines",              &newlines              },
		{ "comments",              &comments              },
		{ "two_tables",            &two_tables            },
		{ "table_ary",             &table_ary             },
		{ "table_with_ary",        &table_with_ary        },
		{ "inline_ary",            &inline_ary            },
		{ "inline_obj",            &inline_obj            },
		{ "empty_inline_ary",      &empty_inline_ary      },
		{ "empty_inline_obj",      &empty_inline_obj      },
		{ "inline_ary_obj",        &inline_ary_obj        },
		{ "inline_obj_ary",        &inline_obj_ary        },
		{ "err_mixed_inline_list", &err_mixed_inline_list },
	};

	return tcase_build_suite("grammar", tests, sizeof(tests));
}
