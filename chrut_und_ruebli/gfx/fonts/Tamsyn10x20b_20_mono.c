
#include "Tamsyn10x20b_20_mono.h"

static const uint32_t chars_data_Tamsyn10x20b_20_mono[] = {
	/* ' ' */
	/* '!' */
	0x0066fff6, 0x00000660,
	/* '"' */
	0x00cf3cf3,
	/* '#' */
	0x99866198, 0xc330ccff, 0x99ff330c, 0x00198661,
	/* '$' */
	0x37c60c18, 0x607c0c1e, 0x60c7d8f0, 0x00000030,
	/* '%' */
	0x736f360e, 0x3b030303, 0x01c1b3db,
	/* '&' */
	0xd86c360e, 0xb6c1c0e0, 0x79e66379, 0x00000006,
	/* ''' */
	0x000000ff,
	/* '(' */
	0xc6331998, 0xc3186318, 0x00000030,
	/* ')' */
	0x318630c3, 0x663318c6, 0x00000006,
	/* '*' */
	0x3c7edb18, 0x0018db7e,
	/* '+' */
	0xff181818, 0x00181818,
	/* ',' */
	0x0007ccee,
	/* '-' */
	0x000000ff,
	/* '.' */
	0x0000003f,
	/* '/' */
	0x6060c0c0, 0x18183030, 0x06060c0c, 0x00000303,
	/* '0' */
	0xbe78f1be, 0x63c79f7f, 0x0000001f,
	/* '1' */
	0x1b1e1c18, 0x18181818, 0x0000ff18,
	/* '2' */
	0x0c1831be, 0x830c30c3, 0x0000003f,
	/* '3' */
	0xc30c307f, 0x63c18303, 0x0000001f,
	/* '4' */
	0x6c787060, 0x60ff6366, 0x00006060,
	/* '5' */
	0xf060c1ff, 0x63c18303, 0x0000001f,
	/* '6' */
	0xf060c31c, 0x63c78f1b, 0x0000001f,
	/* '7' */
	0x860c307f, 0x061830c1, 0x00000003,
	/* '8' */
	0xec78f1be, 0x63c78f1b, 0x0000001f,
	/* '9' */
	0x3c78f1be, 0x30c183f6, 0x0000000e,
	/* ':' */
	0x00fc003f,
	/* ';' */
	0xee0000ee, 0x000007cc,
	/* '<' */
	0x8318c630, 0x003060c1,
	/* '=' */
	0xf000007f, 0x00000007,
	/* '>' */
	0x3060c183, 0x000318c6,
	/* '?' */
	0x60c0c37e, 0x00001830, 0x00181800,
	/* '@' */
	0x1e0d8c7c, 0xe6f37f3c, 0x0fe79bcd, 0x1f806018,
	/* 'A' */
	0x3c78df1c, 0xe3c78ffe, 0x00000031,
	/* 'B' */
	0xfc78f1bf, 0xe3c78f1b, 0x0000001f,
	/* 'C' */
	0x3060c37c, 0x06060c18, 0x0000003e,
	/* 'D' */
	0x3c78d99f, 0xb3c78f1e, 0x0000000f,
	/* 'E' */
	0xf060c1ff, 0x83060c1b, 0x0000003f,
	/* 'F' */
	0xf060c1ff, 0x83060c1b, 0x00000001,
	/* 'G' */
	0x3060c37c, 0x66c78f98, 0x0000003e,
	/* 'H' */
	0xfc78f1e3, 0xe3c78f1f, 0x00000031,
	/* 'I' */
	0x181818ff, 0x18181818, 0x0000ff18,
	/* 'J' */
	0x0c183060, 0x63c78f06, 0x0000001f,
	/* 'K' */
	0x1b3363c3, 0x331b0f0f, 0x0000c363,
	/* 'L' */
	0x3060c183, 0x83060c18, 0x0000003f,
	/* 'M' */
	0xdbffe7c3, 0xc3c3c3c3, 0x0000c3c3,
	/* 'N' */
	0xdbcfc7c3, 0xc3c3e3f3, 0x0000c3c3,
	/* 'O' */
	0x3c78f1be, 0x63c78f1e, 0x0000001f,
	/* 'P' */
	0x3c78f1bf, 0x83060dfe, 0x00000001,
	/* 'Q' */
	0x3c78f1be, 0x63c78f1e, 0x0606061f,
	/* 'R' */
	0x3c78f1bf, 0xe3666dfe, 0x00000031,
	/* 'S' */
	0xe060c1fe, 0xe0c181c0, 0x0000001f,
	/* 'T' */
	0x181818ff, 0x18181818, 0x00001818,
	/* 'U' */
	0x3c78f1e3, 0x63c78f1e, 0x0000001f,
	/* 'V' */
	0x3c78f1e3, 0x1c6cdb1e, 0x0000000e,
	/* 'W' */
	0xc3c3c3c3, 0xffdbdbc3, 0x0000c3e7,
	/* 'X' */
	0xc6d8f1e3, 0xe3c6d8e1, 0x00000031,
	/* 'Y' */
	0x66c3c3c3, 0x1818183c, 0x00001818,
	/* 'Z' */
	0x8618307f, 0x83061861, 0x0000003f,
	/* '[' */
	0xc30c30ff, 0x30c30c30, 0x000fc30c,
	/* '\' */
	0x06060303, 0x18180c0c, 0x60603030, 0x0000c0c0,
	/* ']' */
	0x30c30c3f, 0x0c30c30c, 0x000ff0c3,
	/* '^' */
	0x0c6d8e08,
	/* '_' */
	0x000003ff,
	/* '`' */
	0x000c30c3,
	/* 'a' */
	0x3fd8303e, 0x00fd8f1e,
	/* 'b' */
	0xf060c183, 0xe3c78f1b, 0x0007f8f1,
	/* 'c' */
	0x3060f1be, 0x007d8c18,
	/* 'd' */
	0xec183060, 0xe3c78f1f, 0x000fd8f1,
	/* 'e' */
	0x3ff8f1be, 0x00fc0c18,
	/* 'f' */
	0xf1830678, 0x0c183067, 0x00018306,
	/* 'g' */
	0x3c78f1fe, 0x60fd8f1e, 0x0007d830,
	/* 'h' */
	0xf060c183, 0xe3c78f1b, 0x000c78f1,
	/* 'i' */
	0x00001818, 0x1818181f, 0xff181818,
	/* 'j' */
	0x3e000c30, 0x0c30c30c, 0x7f0c30c3,
	/* 'k' */
	0x03030303, 0x0f1b3363, 0xc363331f,
	/* 'l' */
	0x1818181f, 0x18181818, 0xff181818,
	/* 'm' */
	0xbd7af5ff, 0x00d7af5e,
	/* 'n' */
	0x3c78f1bf, 0x00c78f1e,
	/* 'o' */
	0x3c78f1be, 0x007d8f1e,
	/* 'p' */
	0x3c78f1bf, 0x837f8f1e, 0x000060c1,
	/* 'q' */
	0x3c78f1fe, 0x60fd8f1e, 0x000c1830,
	/* 'r' */
	0x3060c3fb, 0x00060c18,
	/* 's' */
	0x81c0c1fe, 0x007f8303,
	/* 't' */
	0xc19fc60c, 0x0c183060, 0x0000003c,
	/* 'u' */
	0x3c78f1e3, 0x00fd8f1e,
	/* 'v' */
	0x6c78f1e3, 0x003871b3,
	/* 'w' */
	0xc3c3c3c3, 0xe7ffdbdb,
	/* 'x' */
	0xc38db1e3, 0x00c78db1,
	/* 'y' */
	0x3c78f1e3, 0x60fd8f1e, 0x0007d830,
	/* 'z' */
	0xc30c307f, 0x00fe0c30,
	/* '{' */
	0x181818f0, 0x070c1818, 0x1818180c, 0x00f01818,
	/* '|' */
	0x03ffffff,
	/* '}' */
	0x1818180f, 0xe0301818, 0x18181830, 0x000f1818,
	/* '~' */
	0x63f3cfc6,
	/* '¡' */
	0xf6600066, 0x000006ff,
	/* '¢' */
	0x3ccf0c18, 0x66060c18, 0x0003061e,
	/* '£' */
	0xf0c1b33c, 0xee0c1831, 0x0000001d,
	/* '¥' */
	0x183c66c3, 0x7e18187e, 0x00001818,
	/* '«' */
	0x99999998, 0x66198661,
	/* '°' */
	0x1ecf3cde,
	/* '»' */
	0xc330cc33, 0x0ccccccc,
	/* '¿' */
	0x00001818, 0x060c1800, 0x007ec303,
	/* 'Ä' */
	0xe38019b3, 0xffc78f1b, 0x063c78f1,
	/* 'Å' */
	0xc38d9b1c, 0xe3c78db1, 0x063c78ff,
	/* 'Æ' */
	0x98d879f8, 0x8fc66f31, 0x03e63319,
	/* 'Ç' */
	0x3060c37c, 0x06060c18, 0x7183063e, 0x00000000,
	/* 'É' */
	0xf0061860, 0xbf060c1f, 0xf83060c1, 0x00000003,
	/* 'Ñ' */
	0x00336b66, 0xdbcfc7c3, 0xc3c3e3f3, 0x0000c3c3,
	/* 'Ö' */
	0x37c019b3, 0xe3c78f1e, 0x03ec78f1,
	/* 'Ü' */
	0x3c6019b3, 0xe3c78f1e, 0x03ec78f1,
	/* 'ß' */
	0xb678f1be, 0xe3c78f1b, 0x00000019,
	/* 'à' */
	0x00030303, 0xfec181f0, 0x07ec78f1,
	/* 'á' */
	0x00061860, 0xfec181f0, 0x07ec78f1,
	/* 'â' */
	0x000d8e08, 0xfec181f0, 0x07ec78f1,
	/* 'ä' */
	0xe0003366, 0xe3fd8303, 0x000fd8f1,
	/* 'å' */
	0x038d9b1c, 0xfec181f0, 0x07ec78f1,
	/* 'æ' */
	0xb30cc1de, 0x30cc33ff, 0x0000fb83,
	/* 'ç' */
	0x3060f1be, 0x187d8c18, 0x0001c60c,
	/* 'è' */
	0x00030303, 0xffc78df0, 0x07e060c1,
	/* 'é' */
	0x00061860, 0xffc78df0, 0x07e060c1,
	/* 'ê' */
	0x000d8e08, 0xffc78df0, 0x07e060c1,
	/* 'ë' */
	0xe0003366, 0x83ff8f1b, 0x000fc0c1,
	/* 'ì' */
	0x00180c06, 0x18181f00, 0x18181818, 0x000000ff,
	/* 'í' */
	0x00183060, 0x18181f00, 0x18181818, 0x000000ff,
	/* 'î' */
	0x00361c08, 0x18181f00, 0x18181818, 0x000000ff,
	/* 'ï' */
	0x00003333, 0x1818181f, 0xff181818,
	/* 'ñ' */
	0x000cf5e6, 0xe3c78df8, 0x063c78f1,
	/* 'ò' */
	0x00060606, 0xe3c78df0, 0x03ec78f1,
	/* 'ó' */
	0x00061860, 0xe3c78df0, 0x03ec78f1,
	/* 'ô' */
	0x000d8e08, 0xe3c78df0, 0x03ec78f1,
	/* 'ö' */
	0xe00019b3, 0xe3c78f1b, 0x0007d8f1,
	/* '÷' */
	0x00001818, 0x180000ff, 0x00000018,
	/* 'ù' */
	0x00030303, 0xe3c78f18, 0x07ec78f1,
	/* 'ú' */
	0x00061860, 0xe3c78f18, 0x07ec78f1,
	/* 'û' */
	0x000d8e08, 0xe3c78f18, 0x07ec78f1,
	/* 'ü' */
	0x300019b3, 0xe3c78f1e, 0x000fd8f1,
	/* 'ÿ' */
	0x300019b3, 0xe3c78f1e, 0x060fd8f1, 0x000000fb
};

static const char_t chars_Tamsyn10x20b_20_mono[] = {
	{ .utf8_value=0x00000020, .bbox={0}, .data=NULL },
	{ .utf8_value=0x00000021, .bbox={ 2, 3, 6,14}, .data=&chars_data_Tamsyn10x20b_20_mono[0] },
	{ .utf8_value=0x00000022, .bbox={ 1, 2, 7, 6}, .data=&chars_data_Tamsyn10x20b_20_mono[2] },
	{ .utf8_value=0x00000023, .bbox={ 0, 2,10,14}, .data=&chars_data_Tamsyn10x20b_20_mono[3] },
	{ .utf8_value=0x00000024, .bbox={ 1, 1, 8,16}, .data=&chars_data_Tamsyn10x20b_20_mono[7] },
	{ .utf8_value=0x00000025, .bbox={ 0, 4, 9,14}, .data=&chars_data_Tamsyn10x20b_20_mono[11] },
	{ .utf8_value=0x00000026, .bbox={ 0, 3, 9,14}, .data=&chars_data_Tamsyn10x20b_20_mono[14] },
	{ .utf8_value=0x00000027, .bbox={ 3, 2, 5, 6}, .data=&chars_data_Tamsyn10x20b_20_mono[18] },
	{ .utf8_value=0x00000028, .bbox={ 2, 3, 7,17}, .data=&chars_data_Tamsyn10x20b_20_mono[19] },
	{ .utf8_value=0x00000029, .bbox={ 2, 3, 7,17}, .data=&chars_data_Tamsyn10x20b_20_mono[22] },
	{ .utf8_value=0x0000002A, .bbox={ 1, 5, 9,12}, .data=&chars_data_Tamsyn10x20b_20_mono[25] },
	{ .utf8_value=0x0000002B, .bbox={ 1, 5, 9,12}, .data=&chars_data_Tamsyn10x20b_20_mono[27] },
	{ .utf8_value=0x0000002C, .bbox={ 2,12, 6,17}, .data=&chars_data_Tamsyn10x20b_20_mono[29] },
	{ .utf8_value=0x0000002D, .bbox={ 1, 8, 9, 9}, .data=&chars_data_Tamsyn10x20b_20_mono[30] },
	{ .utf8_value=0x0000002E, .bbox={ 3,12, 6,14}, .data=&chars_data_Tamsyn10x20b_20_mono[31] },
	{ .utf8_value=0x0000002F, .bbox={ 1, 2, 9,16}, .data=&chars_data_Tamsyn10x20b_20_mono[32] },
	{ .utf8_value=0x00000030, .bbox={ 1, 4, 8,14}, .data=&chars_data_Tamsyn10x20b_20_mono[36] },
	{ .utf8_value=0x00000031, .bbox={ 1, 4, 9,14}, .data=&chars_data_Tamsyn10x20b_20_mono[39] },
	{ .utf8_value=0x00000032, .bbox={ 1, 4, 8,14}, .data=&chars_data_Tamsyn10x20b_20_mono[42] },
	{ .utf8_value=0x00000033, .bbox={ 1, 4, 8,14}, .data=&chars_data_Tamsyn10x20b_20_mono[45] },
	{ .utf8_value=0x00000034, .bbox={ 1, 4, 9,14}, .data=&chars_data_Tamsyn10x20b_20_mono[48] },
	{ .utf8_value=0x00000035, .bbox={ 1, 4, 8,14}, .data=&chars_data_Tamsyn10x20b_20_mono[51] },
	{ .utf8_value=0x00000036, .bbox={ 1, 4, 8,14}, .data=&chars_data_Tamsyn10x20b_20_mono[54] },
	{ .utf8_value=0x00000037, .bbox={ 1, 4, 8,14}, .data=&chars_data_Tamsyn10x20b_20_mono[57] },
	{ .utf8_value=0x00000038, .bbox={ 1, 4, 8,14}, .data=&chars_data_Tamsyn10x20b_20_mono[60] },
	{ .utf8_value=0x00000039, .bbox={ 1, 4, 8,14}, .data=&chars_data_Tamsyn10x20b_20_mono[63] },
	{ .utf8_value=0x0000003A, .bbox={ 3, 6, 6,14}, .data=&chars_data_Tamsyn10x20b_20_mono[66] },
	{ .utf8_value=0x0000003B, .bbox={ 2, 6, 6,17}, .data=&chars_data_Tamsyn10x20b_20_mono[67] },
	{ .utf8_value=0x0000003C, .bbox={ 2, 4, 8,13}, .data=&chars_data_Tamsyn10x20b_20_mono[69] },
	{ .utf8_value=0x0000003D, .bbox={ 1, 6, 8,11}, .data=&chars_data_Tamsyn10x20b_20_mono[71] },
	{ .utf8_value=0x0000003E, .bbox={ 1, 4, 7,13}, .data=&chars_data_Tamsyn10x20b_20_mono[73] },
	{ .utf8_value=0x0000003F, .bbox={ 1, 3, 9,14}, .data=&chars_data_Tamsyn10x20b_20_mono[75] },
	{ .utf8_value=0x00000040, .bbox={ 0, 2, 9,16}, .data=&chars_data_Tamsyn10x20b_20_mono[78] },
	{ .utf8_value=0x00000041, .bbox={ 1, 4, 8,14}, .data=&chars_data_Tamsyn10x20b_20_mono[82] },
	{ .utf8_value=0x00000042, .bbox={ 1, 4, 8,14}, .data=&chars_data_Tamsyn10x20b_20_mono[85] },
	{ .utf8_value=0x00000043, .bbox={ 1, 4, 8,14}, .data=&chars_data_Tamsyn10x20b_20_mono[88] },
	{ .utf8_value=0x00000044, .bbox={ 1, 4, 8,14}, .data=&chars_data_Tamsyn10x20b_20_mono[91] },
	{ .utf8_value=0x00000045, .bbox={ 1, 4, 8,14}, .data=&chars_data_Tamsyn10x20b_20_mono[94] },
	{ .utf8_value=0x00000046, .bbox={ 1, 4, 8,14}, .data=&chars_data_Tamsyn10x20b_20_mono[97] },
	{ .utf8_value=0x00000047, .bbox={ 1, 4, 8,14}, .data=&chars_data_Tamsyn10x20b_20_mono[100] },
	{ .utf8_value=0x00000048, .bbox={ 1, 4, 8,14}, .data=&chars_data_Tamsyn10x20b_20_mono[103] },
	{ .utf8_value=0x00000049, .bbox={ 1, 4, 9,14}, .data=&chars_data_Tamsyn10x20b_20_mono[106] },
	{ .utf8_value=0x0000004A, .bbox={ 1, 4, 8,14}, .data=&chars_data_Tamsyn10x20b_20_mono[109] },
	{ .utf8_value=0x0000004B, .bbox={ 1, 4, 9,14}, .data=&chars_data_Tamsyn10x20b_20_mono[112] },
	{ .utf8_value=0x0000004C, .bbox={ 1, 4, 8,14}, .data=&chars_data_Tamsyn10x20b_20_mono[115] },
	{ .utf8_value=0x0000004D, .bbox={ 1, 4, 9,14}, .data=&chars_data_Tamsyn10x20b_20_mono[118] },
	{ .utf8_value=0x0000004E, .bbox={ 1, 4, 9,14}, .data=&chars_data_Tamsyn10x20b_20_mono[121] },
	{ .utf8_value=0x0000004F, .bbox={ 1, 4, 8,14}, .data=&chars_data_Tamsyn10x20b_20_mono[124] },
	{ .utf8_value=0x00000050, .bbox={ 1, 4, 8,14}, .data=&chars_data_Tamsyn10x20b_20_mono[127] },
	{ .utf8_value=0x00000051, .bbox={ 1, 4, 8,17}, .data=&chars_data_Tamsyn10x20b_20_mono[130] },
	{ .utf8_value=0x00000052, .bbox={ 1, 4, 8,14}, .data=&chars_data_Tamsyn10x20b_20_mono[133] },
	{ .utf8_value=0x00000053, .bbox={ 1, 4, 8,14}, .data=&chars_data_Tamsyn10x20b_20_mono[136] },
	{ .utf8_value=0x00000054, .bbox={ 1, 4, 9,14}, .data=&chars_data_Tamsyn10x20b_20_mono[139] },
	{ .utf8_value=0x00000055, .bbox={ 1, 4, 8,14}, .data=&chars_data_Tamsyn10x20b_20_mono[142] },
	{ .utf8_value=0x00000056, .bbox={ 1, 4, 8,14}, .data=&chars_data_Tamsyn10x20b_20_mono[145] },
	{ .utf8_value=0x00000057, .bbox={ 1, 4, 9,14}, .data=&chars_data_Tamsyn10x20b_20_mono[148] },
	{ .utf8_value=0x00000058, .bbox={ 1, 4, 8,14}, .data=&chars_data_Tamsyn10x20b_20_mono[151] },
	{ .utf8_value=0x00000059, .bbox={ 1, 4, 9,14}, .data=&chars_data_Tamsyn10x20b_20_mono[154] },
	{ .utf8_value=0x0000005A, .bbox={ 1, 4, 8,14}, .data=&chars_data_Tamsyn10x20b_20_mono[157] },
	{ .utf8_value=0x0000005B, .bbox={ 2, 3, 8,17}, .data=&chars_data_Tamsyn10x20b_20_mono[160] },
	{ .utf8_value=0x0000005C, .bbox={ 1, 2, 9,16}, .data=&chars_data_Tamsyn10x20b_20_mono[163] },
	{ .utf8_value=0x0000005D, .bbox={ 1, 3, 7,17}, .data=&chars_data_Tamsyn10x20b_20_mono[167] },
	{ .utf8_value=0x0000005E, .bbox={ 1, 3, 8, 7}, .data=&chars_data_Tamsyn10x20b_20_mono[170] },
	{ .utf8_value=0x0000005F, .bbox={ 0,16,10,17}, .data=&chars_data_Tamsyn10x20b_20_mono[171] },
	{ .utf8_value=0x00000060, .bbox={ 2, 2, 7, 6}, .data=&chars_data_Tamsyn10x20b_20_mono[172] },
	{ .utf8_value=0x00000061, .bbox={ 1, 6, 8,14}, .data=&chars_data_Tamsyn10x20b_20_mono[173] },
	{ .utf8_value=0x00000062, .bbox={ 1, 2, 8,14}, .data=&chars_data_Tamsyn10x20b_20_mono[175] },
	{ .utf8_value=0x00000063, .bbox={ 1, 6, 8,14}, .data=&chars_data_Tamsyn10x20b_20_mono[178] },
	{ .utf8_value=0x00000064, .bbox={ 1, 2, 8,14}, .data=&chars_data_Tamsyn10x20b_20_mono[180] },
	{ .utf8_value=0x00000065, .bbox={ 1, 6, 8,14}, .data=&chars_data_Tamsyn10x20b_20_mono[183] },
	{ .utf8_value=0x00000066, .bbox={ 1, 2, 8,14}, .data=&chars_data_Tamsyn10x20b_20_mono[185] },
	{ .utf8_value=0x00000067, .bbox={ 1, 6, 8,18}, .data=&chars_data_Tamsyn10x20b_20_mono[188] },
	{ .utf8_value=0x00000068, .bbox={ 1, 2, 8,14}, .data=&chars_data_Tamsyn10x20b_20_mono[191] },
	{ .utf8_value=0x00000069, .bbox={ 1, 2, 9,14}, .data=&chars_data_Tamsyn10x20b_20_mono[194] },
	{ .utf8_value=0x0000006A, .bbox={ 1, 2, 7,18}, .data=&chars_data_Tamsyn10x20b_20_mono[197] },
	{ .utf8_value=0x0000006B, .bbox={ 1, 2, 9,14}, .data=&chars_data_Tamsyn10x20b_20_mono[200] },
	{ .utf8_value=0x0000006C, .bbox={ 1, 2, 9,14}, .data=&chars_data_Tamsyn10x20b_20_mono[203] },
	{ .utf8_value=0x0000006D, .bbox={ 1, 6, 8,14}, .data=&chars_data_Tamsyn10x20b_20_mono[206] },
	{ .utf8_value=0x0000006E, .bbox={ 1, 6, 8,14}, .data=&chars_data_Tamsyn10x20b_20_mono[208] },
	{ .utf8_value=0x0000006F, .bbox={ 1, 6, 8,14}, .data=&chars_data_Tamsyn10x20b_20_mono[210] },
	{ .utf8_value=0x00000070, .bbox={ 1, 6, 8,18}, .data=&chars_data_Tamsyn10x20b_20_mono[212] },
	{ .utf8_value=0x00000071, .bbox={ 1, 6, 8,18}, .data=&chars_data_Tamsyn10x20b_20_mono[215] },
	{ .utf8_value=0x00000072, .bbox={ 1, 6, 8,14}, .data=&chars_data_Tamsyn10x20b_20_mono[218] },
	{ .utf8_value=0x00000073, .bbox={ 1, 6, 8,14}, .data=&chars_data_Tamsyn10x20b_20_mono[220] },
	{ .utf8_value=0x00000074, .bbox={ 1, 4, 8,14}, .data=&chars_data_Tamsyn10x20b_20_mono[222] },
	{ .utf8_value=0x00000075, .bbox={ 1, 6, 8,14}, .data=&chars_data_Tamsyn10x20b_20_mono[225] },
	{ .utf8_value=0x00000076, .bbox={ 1, 6, 8,14}, .data=&chars_data_Tamsyn10x20b_20_mono[227] },
	{ .utf8_value=0x00000077, .bbox={ 0, 6, 8,14}, .data=&chars_data_Tamsyn10x20b_20_mono[229] },
	{ .utf8_value=0x00000078, .bbox={ 1, 6, 8,14}, .data=&chars_data_Tamsyn10x20b_20_mono[231] },
	{ .utf8_value=0x00000079, .bbox={ 1, 6, 8,18}, .data=&chars_data_Tamsyn10x20b_20_mono[233] },
	{ .utf8_value=0x0000007A, .bbox={ 1, 6, 8,14}, .data=&chars_data_Tamsyn10x20b_20_mono[236] },
	{ .utf8_value=0x0000007B, .bbox={ 1, 2, 9,17}, .data=&chars_data_Tamsyn10x20b_20_mono[238] },
	{ .utf8_value=0x0000007C, .bbox={ 3, 3, 5,16}, .data=&chars_data_Tamsyn10x20b_20_mono[242] },
	{ .utf8_value=0x0000007D, .bbox={ 1, 2, 9,17}, .data=&chars_data_Tamsyn10x20b_20_mono[243] },
	{ .utf8_value=0x0000007E, .bbox={ 0, 3, 8, 7}, .data=&chars_data_Tamsyn10x20b_20_mono[247] },
	{ .utf8_value=0x000000A1, .bbox={ 2, 6, 6,17}, .data=&chars_data_Tamsyn10x20b_20_mono[248] },
	{ .utf8_value=0x000000A2, .bbox={ 1, 3, 8,15}, .data=&chars_data_Tamsyn10x20b_20_mono[250] },
	{ .utf8_value=0x000000A3, .bbox={ 1, 4, 8,14}, .data=&chars_data_Tamsyn10x20b_20_mono[253] },
	{ .utf8_value=0x000000A5, .bbox={ 1, 4, 9,14}, .data=&chars_data_Tamsyn10x20b_20_mono[256] },
	{ .utf8_value=0x000000AB, .bbox={ 0, 6, 9,13}, .data=&chars_data_Tamsyn10x20b_20_mono[259] },
	{ .utf8_value=0x000000B0, .bbox={ 1, 4, 7, 9}, .data=&chars_data_Tamsyn10x20b_20_mono[261] },
	{ .utf8_value=0x000000BB, .bbox={ 0, 6, 9,13}, .data=&chars_data_Tamsyn10x20b_20_mono[262] },
	{ .utf8_value=0x000000BF, .bbox={ 1, 6, 9,17}, .data=&chars_data_Tamsyn10x20b_20_mono[264] },
	{ .utf8_value=0x000000C4, .bbox={ 1, 1, 8,14}, .data=&chars_data_Tamsyn10x20b_20_mono[267] },
	{ .utf8_value=0x000000C5, .bbox={ 1, 1, 8,14}, .data=&chars_data_Tamsyn10x20b_20_mono[270] },
	{ .utf8_value=0x000000C6, .bbox={ 0, 4, 9,14}, .data=&chars_data_Tamsyn10x20b_20_mono[273] },
	{ .utf8_value=0x000000C7, .bbox={ 1, 4, 8,18}, .data=&chars_data_Tamsyn10x20b_20_mono[276] },
	{ .utf8_value=0x000000C9, .bbox={ 1, 0, 8,14}, .data=&chars_data_Tamsyn10x20b_20_mono[280] },
	{ .utf8_value=0x000000D1, .bbox={ 1, 0, 9,14}, .data=&chars_data_Tamsyn10x20b_20_mono[284] },
	{ .utf8_value=0x000000D6, .bbox={ 1, 1, 8,14}, .data=&chars_data_Tamsyn10x20b_20_mono[288] },
	{ .utf8_value=0x000000DC, .bbox={ 1, 1, 8,14}, .data=&chars_data_Tamsyn10x20b_20_mono[291] },
	{ .utf8_value=0x000000DF, .bbox={ 1, 4, 8,14}, .data=&chars_data_Tamsyn10x20b_20_mono[294] },
	{ .utf8_value=0x000000E0, .bbox={ 1, 1, 8,14}, .data=&chars_data_Tamsyn10x20b_20_mono[297] },
	{ .utf8_value=0x000000E1, .bbox={ 1, 1, 8,14}, .data=&chars_data_Tamsyn10x20b_20_mono[300] },
	{ .utf8_value=0x000000E2, .bbox={ 1, 1, 8,14}, .data=&chars_data_Tamsyn10x20b_20_mono[303] },
	{ .utf8_value=0x000000E4, .bbox={ 1, 2, 8,14}, .data=&chars_data_Tamsyn10x20b_20_mono[306] },
	{ .utf8_value=0x000000E5, .bbox={ 1, 1, 8,14}, .data=&chars_data_Tamsyn10x20b_20_mono[309] },
	{ .utf8_value=0x000000E6, .bbox={ 0, 6,10,14}, .data=&chars_data_Tamsyn10x20b_20_mono[312] },
	{ .utf8_value=0x000000E7, .bbox={ 1, 6, 8,18}, .data=&chars_data_Tamsyn10x20b_20_mono[315] },
	{ .utf8_value=0x000000E8, .bbox={ 1, 1, 8,14}, .data=&chars_data_Tamsyn10x20b_20_mono[318] },
	{ .utf8_value=0x000000E9, .bbox={ 1, 1, 8,14}, .data=&chars_data_Tamsyn10x20b_20_mono[321] },
	{ .utf8_value=0x000000EA, .bbox={ 1, 1, 8,14}, .data=&chars_data_Tamsyn10x20b_20_mono[324] },
	{ .utf8_value=0x000000EB, .bbox={ 1, 2, 8,14}, .data=&chars_data_Tamsyn10x20b_20_mono[327] },
	{ .utf8_value=0x000000EC, .bbox={ 1, 1, 9,14}, .data=&chars_data_Tamsyn10x20b_20_mono[330] },
	{ .utf8_value=0x000000ED, .bbox={ 1, 1, 9,14}, .data=&chars_data_Tamsyn10x20b_20_mono[334] },
	{ .utf8_value=0x000000EE, .bbox={ 1, 1, 9,14}, .data=&chars_data_Tamsyn10x20b_20_mono[338] },
	{ .utf8_value=0x000000EF, .bbox={ 1, 2, 9,14}, .data=&chars_data_Tamsyn10x20b_20_mono[342] },
	{ .utf8_value=0x000000F1, .bbox={ 1, 1, 8,14}, .data=&chars_data_Tamsyn10x20b_20_mono[345] },
	{ .utf8_value=0x000000F2, .bbox={ 1, 1, 8,14}, .data=&chars_data_Tamsyn10x20b_20_mono[348] },
	{ .utf8_value=0x000000F3, .bbox={ 1, 1, 8,14}, .data=&chars_data_Tamsyn10x20b_20_mono[351] },
	{ .utf8_value=0x000000F4, .bbox={ 1, 1, 8,14}, .data=&chars_data_Tamsyn10x20b_20_mono[354] },
	{ .utf8_value=0x000000F6, .bbox={ 1, 2, 8,14}, .data=&chars_data_Tamsyn10x20b_20_mono[357] },
	{ .utf8_value=0x000000F7, .bbox={ 1, 4, 9,13}, .data=&chars_data_Tamsyn10x20b_20_mono[360] },
	{ .utf8_value=0x000000F9, .bbox={ 1, 1, 8,14}, .data=&chars_data_Tamsyn10x20b_20_mono[363] },
	{ .utf8_value=0x000000FA, .bbox={ 1, 1, 8,14}, .data=&chars_data_Tamsyn10x20b_20_mono[366] },
	{ .utf8_value=0x000000FB, .bbox={ 1, 1, 8,14}, .data=&chars_data_Tamsyn10x20b_20_mono[369] },
	{ .utf8_value=0x000000FC, .bbox={ 1, 2, 8,14}, .data=&chars_data_Tamsyn10x20b_20_mono[372] },
	{ .utf8_value=0x000000FF, .bbox={ 1, 2, 8,17}, .data=&chars_data_Tamsyn10x20b_20_mono[375] }
};

const font_t font_Tamsyn10x20b_20_mono = {
	.fontsize       = 20,
	.lineheight     = 20,
	.ascender       = 14,
	.descender      = -6,
	.charwidth      = 10,
	.char_count     = 138,
	.chars          = chars_Tamsyn10x20b_20_mono,
	.chars_data     = chars_data_Tamsyn10x20b_20_mono,
};

