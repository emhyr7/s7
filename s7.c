#include <assert.h>
#include <stdarg.h>
#include <stdio.h>

#undef stdin
#undef stdout
#undef stderr

#define exit(...) _exit(__VA_ARGS__)

typedef __UINT8_TYPE__  UINT8;
typedef __UINT16_TYPE__ UINT16;
typedef __UINT32_TYPE__ UINT32;
typedef __UINT64_TYPE__ UINT64;
typedef __INT8_TYPE__   SINT8;
typedef __INT16_TYPE__  SINT16;
typedef __INT32_TYPE__  SINT32;
typedef __INT64_TYPE__  SINT64;
typedef float           FLOAT32;
typedef double          FLOAT64;

typedef UINT8  BYTE;
typedef UINT16 HALF;
typedef UINT32 WORD;
typedef UINT64 LONG;

typedef UINT8 BIT;

typedef va_list VARGS;

#define get_vargs(...)        va_start(__VA_ARGS__)
#define get_varg(type, vargs) va_arg(vargs, type)
#define end_vargs(...)        va_end(__VA_ARGS__)
#define copy_vargs(...)       va_copy(__VA_ARGS__)

/*
	uint64 = *(UINT64 *)&float64;
	UINT32 mantissa = uint64 & 0xfffffffffffff;
	uint64 >>= 52;
	UINT8 exponent = uint64 & 0x7ff;
	uint64 >>= 11;
	BIT sign = uint64 & 1;
*/

typedef char ASCII;

_Noreturn void exit(SINT32 status);

void *allocate(UINT32 size);

typedef void *HANDLE;

HANDLE open_file       (const ASCII *path);
UINT64 get_size_of_file(HANDLE file);
UINT32 read_from_file  (void *buffer, UINT32 size, HANDLE file);
UINT32 write_into_file (const void *buffer, UINT32 size, HANDLE file);

extern HANDLE stdin;
extern HANDLE stdout;
extern HANDLE stderr;

/*
	The following formats are supported:
		%% - '%'
		%t - TEXT
		%f - FLOAT64
		%b - SINT8
		%h - SINT16
		%w - SINT32
		%l - SINT64
		%u - UINT64
*/
ASCII *format_v(ASCII *buffer, const ASCII *format, VARGS vargs)
{
	/* TODO: if `!buffer`, don't write, but keep incrementing to return
	   what would be the end of the formatted text if `buffer` wasn't null */

	ASCII *caret = buffer;

	while(*format)
	{
		ASCII byte = *format++;
		if(byte == '%')
		{
			ASCII   *text;
			UINT64   uint64;
			SINT64   sint64;
			FLOAT64  float64;
			ASCII    temporary[64];
			
			ASCII specifier = *format++;
			switch(specifier)
			{
			case '%':
				*caret++ = '%';
				break;
			case 't':
				text = get_varg(ASCII *, vargs);
			print_text:
				while(*text) *caret++ = *text++;
				break;
			case 'f':
				float64 = get_varg(FLOAT64, vargs);
				*caret++ = *(UINT64 *)(&float64) & 0x8000000000000000 ? '-' : '+';
				UINT64 whole = (UINT64)float64;
				text = temporary + sizeof(temporary) - 1;
				*text-- = 0;
				for(uint64 = (FLOAT64)(float64 - whole) * 1000000; uint64; uint64 /= 10) *text-- = '0' + uint64 % 10;
				*text-- = '.';
				for(; whole; whole /= 10) *text-- = '0' + whole % 10;
				++text;
				goto print_text;
			default:
				uint64 = get_varg(UINT64, vargs);
				switch(specifier)
				{
				case 'b':
					sint64 = (SINT64)*(SINT8 *)&uint64;
					goto print_signed;
				case 'h':
					sint64 = (SINT64)*(SINT16 *)&uint64;
					goto print_signed;
				case 'w':
					sint64 = (SINT64)*(SINT32 *)&uint64;
					goto print_signed;
				case 'l':
					sint64 = *(SINT64 *)&uint64;
				print_signed:
					*caret++ = sint64 & 0x8000000000000000 ? '-' : '+';
					uint64 = ~(sint64 - 1);
					goto print_unsigned;
				case 'u':
				print_unsigned:
					text = temporary + sizeof(temporary) - 1;
					*text-- = 0;
					for(; uint64; uint64 /= 10) *text-- = '0' + uint64 % 10;
					++text;
					goto print_text;
				default:
					assert(!"Shitty ass format!");
					break;
				}
			}
		}
		else *caret++ = byte;
	}

	*caret = 0;
	return caret;
}

ASCII *format(ASCII *buffer, const ASCII *format, ...)
{
	VARGS vargs;
	get_vargs(vargs, format);
	ASCII *caret = format_v(buffer, format, vargs);
	end_vargs(vargs);
	return caret;
}

UINT32 print_v(const ASCII *format, VARGS vargs)
{
	ASCII message[4096];
	ASCII *caret = format_v(message, format, vargs);
	UINT32 written_size = caret - message;
	written_size = write_into_file(message, written_size, stdout);
	return written_size;
}

UINT32 print(const ASCII *format, ...)
{
	VARGS vargs;
	get_vargs(vargs, format);
	UINT32 written_size = print_v(format, vargs);
	end_vargs(vargs);
	return written_size;
}

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
	ASCII *identifier;
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
	ASCII       *identifier;
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

inline BIT check_whitespace(ASCII byte)
{
	return (byte >= '\t' && byte <= '\r')
		|| (byte == ' ');
}

inline BIT check_letter(ASCII byte)
{
	return (byte >= 'A' && byte <= 'Z')
		|| (byte >= 'a' && byte <= 'z');
}

inline BIT check_digit(ASCII byte)
{
	return byte >= '0' && byte <= '9';
}

inline BIT check_binary(ASCII byte)
{
	return byte == '0' || byte == '1';
}

inline BIT check_hex(ASCII byte)
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

BIT tokenize(TOKEN *token, SOURCE_LOCATION *location)
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
		BIT (*number_checker)(ASCII) = &check_digit;
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

typedef struct
{
	TOKEN           token;
	SOURCE_LOCATION location;
} PARSER;

typedef enum
{
	NULL_NODE_TYPE,
} NODE_TYPE;

typedef struct
{
	NODE_TYPE type;
	NODE_TYPE subnodes[]; /* a list of nodes terminated with `null_node` */
} NODE;

const NODE null_node = { NULL_NODE_TYPE };

typedef struct
{
	NODE_TYPE type;
	NODE subnode; /* a list of nodes until `null_node` */
} UNARY_NODE;

typedef struct
{
	NODE_TYPE type;
	NODE subnodes[]; /* two lists of nodes separated with `null_node` */
} BINARY_NODE;

typedef struct
{
	NODE_TYPE type;
	UINT64 count; /* count of lists */
	NODE subnodes[]; /* a list of lists of nodes separated with `null_node` */
} LIST_NODE;

typedef struct
{
} SYNTAX;

typedef UINT8 PRECEDENCE;

void parse_node(PRECEDENCE left_precedence, TOKEN *token)
{
}

SYNTAX *parse(SOURCE *source)
{
	return 0;
}

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

int start(int argc, char *argv[])
{
	ASCII buffer[128];
	print("%t %u %b %f %f\n", "Hello, World!", 7, -21, 9.14, 9.0);

#if 0
	if(argc <= 1)
	{
		print_help();
		return 0;
	}

	SOURCE source;
	load_file(argv[1], &source);
	SOURCE_LOCATION location = { &source, 0, 1, 1 };
	TOKEN token;
	while(tokenize(&token, &location)) report(&(SOURCE_RANGE){&source, token.range}, "token: ");
#endif
	return 0;
}
