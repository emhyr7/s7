#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

typedef __UINT8_TYPE__  UINT8;
typedef __UINT16_TYPE__ UINT16;
typedef __UINT32_TYPE__ UINT32;
typedef __UINT64_TYPE__ UINT64;
typedef __INT8_TYPE__   SINT8;
typedef __INT16_TYPE__  SINT16;
typedef __INT32_TYPE__  SINT32;
typedef __INT64_TYPE__  SINT64;
typedef float           REAL32;
typedef double          REAL64;

typedef va_list VARGS;

#define get_vargs(...)  va_start(__VA_ARGS__)
#define get_varg(...)   va_arg(__VA_ARGS__)
#define end_vargs(...)  va_end(__VA_ARGS__)
#define copy_vargs(...) va_copy(__VA_ARGS__)

typedef char ASCII;

typedef enum : UINT64
{
	NULL_TYPE_CLASS,
	TYPE_CLASS_uint8,
	TYPE_CLASS_uint16,
	TYPE_CLASS_uint32,
	TYPE_CLASS_uint64,
	TYPE_CLASS_sint8,
	TYPE_CLASS_sint16,
	TYPE_CLASS_sint32,
	TYPE_CLASS_sint64,
	TYPE_CLASS_real32,
	TYPE_CLASS_real64,
	TYPE_CLASS_tuple,
	TYPE_CLASS_address,
} TYPE_CLASS;

typedef struct TYPE TYPE;

struct TYPE
{
	TYPE_CLASS class;
	UINT64     multiplier;
	TYPE      *subtypes[]; /* if `class == TYPE_CLASS_tuple` */
};

/* primitive types */
TYPE uint8_type  = { TYPE_CLASS_uint8,  1 };
TYPE uint16_type = { TYPE_CLASS_uint16, 1 };
TYPE uint32_type = { TYPE_CLASS_uint32, 1 };
TYPE uint64_type = { TYPE_CLASS_uint64, 1 };
TYPE sint8_type  = { TYPE_CLASS_sint8,  1 };
TYPE sint16_type = { TYPE_CLASS_sint16, 1 };
TYPE sint32_type = { TYPE_CLASS_sint32, 1 };
TYPE sint64_type = { TYPE_CLASS_sint64, 1 };
TYPE real32_type = { TYPE_CLASS_real32, 1 };
TYPE real64_type = { TYPE_CLASS_real64, 1 };

typedef struct
{
	UINT8 *identifier;
	TYPE  *type;
} VALUE;

const VALUE null_value = { 0, 0 };

typedef enum : UINT64
{
	NULL_OPERATOR,

	/* scope traversal */
	OPERATOR_psh, /* { */
	OPERATOR_pop, /* } */

	/* other */
	OPERATOR_ref, /* @a */
	OPERATOR_jmp, /* 'a */
	OPERATOR_get, /* a . b */
	OPERATOR_set, /* a = b */
	OPERATOR_inv, /* a b */
	OPERATOR_imp, /* a ? b : c */

	/* arithmetic */
	OPERATOR_neg, /* -a */
	OPERATOR_add, /* a + b */
	OPERATOR_sub, /* a - b */
	OPERATOR_mul, /* a * b */
	OPERATOR_div, /* a / b */
	OPERATOR_mod, /* a % b */
	/* OPERATOR_rem, */

	/* bitwise */
	OPERATOR_bng, /* ~a */
	OPERATOR_bor, /* a | b */
	OPERATOR_bnd, /* a & b */
	OPERATOR_bxr, /* a ^ b */
	OPERATOR_lsh, /* a << b */
	OPERATOR_rsh, /* a >> b */

	/* logical */
	OPERATOR_lng, /* !a */
	OPERATOR_lnd, /* a && b */
	OPERATOR_lor, /* a || b */
	OPERATOR_leq, /* a == b */
	OPERATOR_neq, /* a != b */
	OPERATOR_gre, /* a >= b */
	OPERATOR_lre, /* a <= b */
	OPERATOR_lgr, /* a > b */
	OPERATOR_llr, /* a < b */
} OPERATOR;

typedef struct
{
	OPERATOR operator;
	VALUE   *operands[];
} INSTRUCTION;

const INSTRUCTION null_instruction = { NULL_OPERATOR };

typedef struct
{
	UINT8       *identifier;
	VALUE       *values; /* the first `null_value` terminates the arguments,the second `null_value` terminates the results */
	INSTRUCTION *instructions;
} PROCEDURE;

const PROCEDURE null_procedure = { 0, 0, 0 };

typedef struct
{
	VALUE     *values;
	VALUE     *constants;
	PROCEDURE *procedures;
} MODULE;

const MODULE null_module = { 0, 0, 0 };

typedef struct
{
	MODULE *modules;
} PROGRAM;

typedef struct
{
	const ASCII *path;
	ASCII *data;
} SOURCE;

typedef struct
{
	UINT64 beginning;
	UINT64 ending;
	UINT64 row;
	UINT64 column;
} RANGE;

typedef struct
{
	SOURCE *source;
	RANGE   range;
} SOURCE_RANGE;

typedef enum
{
	TOKEN_TYPE_name = 127,
	TOKEN_TYPE_binary,
	TOKEN_TYPE_digital,
	TOKEN_TYPE_hexadecimal,
	TOKEN_TYPE_decimal,
	TOKEN_TYPE_text,
} TOKEN_TYPE;

typedef struct
{
	TOKEN_TYPE type;
	RANGE      range;
} TOKEN;

typedef struct
{
	SOURCE *source;
	UINT64  position;
	UINT64  row;
	UINT64  column;
} SOURCE_LOCATION;

typedef struct
{
	TOKEN token;
} PARSER;

typedef struct
{
} NODE;

typedef struct
{

} SYNTAX;

inline UINT8 check_whitespace(ASCII byte)
{
	return (byte >= '\t' && byte <= '\r')
		|| (byte == ' ');
}

inline UINT8 check_letter(ASCII byte)
{
	return (byte >= 'A' && byte <= 'Z')
		|| (byte >= 'a' && byte <= 'z');
}

inline UINT8 check_digit(ASCII byte)
{
	return byte >= '0' && byte <= '9';
}

inline UINT8 check_binary(ASCII byte)
{
	return byte == '0' || byte == '1';
}

inline UINT8 check_hex(ASCII byte)
{
	return check_digit(byte)
		|| (byte >= 'A' && byte <= 'F')
		|| (byte >= 'a' && byte <= 'f');
}

inline ASCII get_byte(SOURCE_LOCATION *location)
{
	return location->source->data[location->position];
}

inline ASCII peek_byte(SOURCE_LOCATION *location)
{
	return location->source->data[location->position + 1];
}

ASCII advance(SOURCE_LOCATION *location)
{
	if (get_byte(location) == '\n')
	{
		++location->row;
		location->column = 0;
	}
	++location->column;
	++location->position;
	return get_byte(location);
}

void report_v(const SOURCE_RANGE *range, const ASCII *message, VARGS vargs) 
{
	fprintf(stderr, "%s[%llu-%llu|%llu,%llu]: ", range->source->path, range->range.beginning, range->range.ending, range->range.row, range->range.column);
	vfprintf(stderr, message, vargs);
	fputc('\n', stderr);

	UINT64 range_size = range->range.ending - range->range.beginning;
	if(!range_size) return;

	fprintf(stderr, "\t%.*s\n", (int)range_size, range->source->data + range->range.beginning);
	/* TODO: print a view of the source location */
}

void report(const SOURCE_RANGE *range, const ASCII *message, ...)
{
	VARGS vargs;
	get_vargs(vargs, message);
	report_v(range, message, vargs);
	end_vargs(vargs);
}

_Noreturn void fail(const SOURCE_RANGE *range, const ASCII *message, ...)
{
	VARGS vargs;
	get_vargs(vargs, message);
	report_v(range, message, vargs);
	end_vargs(vargs);

	exit(-1);
}

UINT8 tokenize(TOKEN *token, SOURCE_LOCATION *location)
{
	const ASCII *failure_message = 0;

repeat:
	ASCII byte = get_byte(location);

	while(check_whitespace(byte)) byte = advance(location);

	token->range.beginning = location->position;
	token->range.row       = location->row;
	token->range.column    = location->column;

	TOKEN_TYPE token_type;

	if(check_letter(byte) || byte == '_')
	{
		token_type = TOKEN_TYPE_name;
		do byte = advance(location);
		while(check_letter(byte) || byte == '_' || check_digit(byte) || byte == '-');
	}
	else if(check_digit(byte))
	{
		UINT8 (*number_checker)(ASCII) = &check_digit;
		token_type = TOKEN_TYPE_digital;
		if(byte == '0')
		{
			switch(advance(location))
			{
			case 'b':
				token_type = TOKEN_TYPE_binary;
				number_checker = &check_binary;
				advance(location);
				break;
			case 'x':
				token_type = TOKEN_TYPE_hexadecimal;
				number_checker = &check_hex;
				advance(location);
				break;
			default:
				break;
			}
		}

		do
		{
			byte = advance(location);
			if(byte == '.')
			{
				if(token_type == TOKEN_TYPE_decimal
					|| token_type == TOKEN_TYPE_binary
					|| token_type == TOKEN_TYPE_hexadecimal)
				{
					failure_message = "Weird ass number.";
					goto failed;
				}
				token_type = TOKEN_TYPE_decimal;
				byte = advance(location);
			}
		}
		while(check_digit(byte) || byte == '_');
	}
	else switch(byte)
	{
	case '\3':
		return 0;
	case '"':
		for(;;)
		{
			byte = advance(location);
			if(byte == '\\') byte = advance(location);
			else if(byte == '"') break;
		}
		advance(location);
		token_type = TOKEN_TYPE_text;
		break;
	case '#':
		if(peek_byte(location) == ' ')
		{
			do byte = advance(location);
			while(byte != '\n');
			goto repeat;
		}
	case '!':
	case '%':
	case '&':
	case '\'':
	case '(':
	case ')':
	case '*':
	case '+':
	case ',':
	case '-':
	case '.':
	case '/':
	case ':':
	case ';':
	case '<':
	case '=':
	case '>':
	case '?':
	case '@':
	case '[':
	case '\\':
	case ']':
	case '^':
	case '`':
	case '{':
	case '|':
	case '}':
	case '~':
		token_type = byte;
		advance(location);
		break;
	default:
		failure_message = "Unknown character.";
		goto failed;
	}

	token->range.ending = location->position;
	return 1;

failed:
	token->range.ending = location->position;
	fail(&(SOURCE_RANGE){location->source, token->range}, failure_message);
	return 0;
}

SYNTAX *parse(SOURCE_LOCATION *location, PARSER *parser)
{
	/* per expression, construct a discontiguous tree of nodes, then flatten
	   the resulting tree */

	SYNTAX *result = 0;
	return result;
}

typedef void *HANDLE;

HANDLE open_file(const ASCII *path);
UINT64 get_size_of_file(HANDLE file);
UINT32 read_from_file(void *buffer, UINT32 file_size, HANDLE file);
void *allocate(UINT32 size);

void load_file(const ASCII *file_path, SOURCE *source)
{
	source->path = file_path;
	HANDLE file = open_file(file_path);
	UINT64 file_size = get_size_of_file(file);
	source->data = allocate(file_size + 1);
	read_from_file(source->data, file_size, file);
	source->data[file_size] = '\3';
}

void print_help(void)
{
	printf("s7c <source-path>");
}

int main(int argc, char *argv[])
{
	if(argc <= 1)
	{
		print_help();
		return 0;
	}

	SOURCE source;
	load_file(argv[1], &source);
	SOURCE_LOCATION location =
	{
		&source,
		0,
		1,
		1
	};
	TOKEN token;
	while(tokenize(&token, &location))
		report(&(SOURCE_RANGE){&source, token.range}, "token: ");

	return 0;
}
