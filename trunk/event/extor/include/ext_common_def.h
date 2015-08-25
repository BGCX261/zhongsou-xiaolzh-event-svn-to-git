#ifndef ___EXT_COMMON_DEF_H_
#define ___EXT_COMMON_DEF_H_

#include <ctype.h>
#include "UH_Define.h"
#include "utility.h"

//XXX_word_extractor
#define WE_init 0x0001
#define WE_EXTR 0x0002

#define SET_FLAG(x, y)      (x) |= (y)
#define TEST_FLAG(x, y)     (0 != ((x) & (y)))
#define RESET_FLAG(x, y)    ((x) ^= (y))

#define MAX_KEYWORD_LEN (32)
#define MAX_NEWS_FEATURE_NUM (8192)
#define MAX_USED_POS_NUM (1024)

#define DF_MAX_NUM (4<<20)
#define NID_MAX_NUM (30<<20)

typedef int word_type_e;

typedef struct POS_Weight
{
	var_u8 pos_id;
	var_f4 weight;

}tagweight, *ptagweight;

// enum word_type_e
// {
// 	word_type_all,
// 	word_type_noun,
// 	word_type_verb,
// 	word_type_adverb
// };

#define WT_ALL  0x0000
#define WT_NAME 0x0001
#define WT_NOUN 0x0002
#define WT_VERB 0x0004
#define WT_OTHR 0x0010

typedef struct tag_term_info_st
{
    var_u1 tf;

	word_type_e type;
	var_f4 weight;
	
	var_f4 idf;
    var_u8  word_id;
    var_1 word_str[MAX_KEYWORD_LEN + 1];
}term_info_st, *pterm_info_st;

// template<typename T1, typename T2>
// struct simple_pair
// {
//     T1 left;
//     T2 right;
// };

#define RET_FALSE               (1)
#define RET_SECCEED             (0)
#define RET_ERROR_INVALID_PARAM (-1)
#define RET_NO_ENOUGH_MEMORY    (-2)
#define CHECK_FAILED(x) (0 > x)

#ifdef _WIN32_ENV_
#define PATH_2_NAME(x) strrchr(x, '\\') + 1
#else
#define PATH_2_NAME(x) x
#endif

#define _DEBUG_P
#ifdef _DEBUG_P
#define PRINT_DEBUG_INFO printf
#define LOG_TRY(x, y)         printf("\rThread is trying[%s] param[%s]\t\r", x, y)
#define LOG_ERROR(x,y)        printf("FILE: %s LINE: %d\nERROR: [%s] [%s] \n", PATH_2_NAME(__FILE__), __LINE__, x, y)
#define LOG_WARNING(x)      printf("FILE: %s LINE: %d\nWARNING: [%s] \n", PATH_2_NAME(__FILE__), __LINE__, x)
#define LOG_NULL_POINTER(x) printf("FILE: %s LINE: %d\nERROR: [%s] is NULL pointer\n", PATH_2_NAME(__FILE__), __LINE__, x)
#define LOG_INVALID_PARAMETER(x,y) printf("FILE: %s LINE: %d\nERROR: %s parameter is invlalid, it's[[%s]]\n", PATH_2_NAME(__FILE__), __LINE__, x, y)
#define LOG_FAILE_CALL(x,y) printf("FILE: %s LINE: %d\nERROR: in [%s], failed to call [%s]\n",PATH_2_NAME(__FILE__), __LINE__, x, y)
#define LOG_FAILE_CALL_PARAM(x,y,z) printf("FILE: %s LINE: %d\nERROR: in [%s], failed to call [%s], parameter is [%s]\n", PATH_2_NAME(__FILE__), __LINE__, x, y, z)
#define LOG_FAILE_CALL_LEN_PARAM(x,y,l,z) printf("FILE: %s LINE: %d\nERROR: in [%s], failed to call [%s], parameter is [%.*s]\n", PATH_2_NAME(__FILE__), __LINE__, x, y, l, z)
#define LOG_FAILE_CALL_RET(x,y,r) printf("FILE: %s LINE: %d\nERROR: in [%s], failed to call [%s], return [%d]\n", PATH_2_NAME(__FILE__), __LINE__, x, y, r)
#define LOG_FAILE_CALL_ID(x,y,r) printf("FILE: %s LINE: %d\nERROR: in [%s], failed to call [%s], ID "CP_PU64"\n", PATH_2_NAME(__FILE__), __LINE__, x, y, r)
#define LOG_PARAM(x,y,z) printf("FILE: %s LINE: %d\nERROR: in [%s], parameter [%s] is [%s]\n", PATH_2_NAME(__FILE__), __LINE__, x, y, z)
#define LOG_FAILE_NEW(x) printf("FILE: %s LINE: %d\nERROR: failed to new [%s] object\n", PATH_2_NAME(__FILE__), __LINE__, x)
#define LOG_FAILE_DUPSTR(x) printf("FILE: %s LINE: %d\nERROR: failed to duplicate [%s] string\n", PATH_2_NAME(__FILE__), __LINE__, x)
#else
#define PRINT_DEBUG_INFO 
#define LOG_TRY(x, y)
#define LOG_ERROR(x,y)
#define LOG_WARNING(x)
#define LOG_NULL_POINTER(x)
#define LOG_INVALID_PARAMETER(x,y)
#define LOG_FAILE_CALL(x,y)
#define LOG_FAILE_CALL_PARAM(x,y,z)
#define LOG_FAILE_CALL_LEN_PARAM(x,y,l,z)
#define LOG_FAILE_CALL_RET(x,y,r) 
#define LOG_FAILE_CALL_ID(x,y,r) 
#define LOG_PARAM(x,y,z) 
#define LOG_FAILE_NEW(x) 
#define LOG_FAILE_DUPSTR(x) 
#endif

#define NULL_POINTER(x) "FILE: %s LINE: %d\nERROR: [%s] is NULL pointer\n", PATH_2_NAME(__FILE__), __LINE__, x
#define INVALID_PARAMETER(x,y) "FILE: %s LINE: %d\nERROR: %s parameter is invlalid, it's[[%s]]\n", PATH_2_NAME(__FILE__), __LINE__, x, y
#define FAILE_CALL(x,y) "FILE: %s LINE: %d\nERROR: in [%s], failed to call [%s]\n",PATH_2_NAME(__FILE__), __LINE__, x, y
#define FAILE_CALL_PARAM(x,y,z) "FILE: %s LINE: %d\nERROR: in [%s], failed to call [%s], parameter is [%s]\n", PATH_2_NAME(__FILE__), __LINE__, x, y, z
#define FAILE_CALL_LEN_PARAM(x,y,l,z) "FILE: %s LINE: %d\nERROR: in [%s], failed to call [%s], parameter is [%.*s]\n", PATH_2_NAME(__FILE__), __LINE__, x, y, l, z
#define FAILE_CALL_RET(x,y,r) "FILE: %s LINE: %d\nERROR: in [%s], failed to call [%s], return [%d]\n", PATH_2_NAME(__FILE__), __LINE__, x, y, r
#define FAILE_CALL_ID(x,y,r) "FILE: %s LINE: %d\nERROR: in [%s], failed to call [%s], ID "CP_PU64"\n", PATH_2_NAME(__FILE__), __LINE__, x, y, r
#define PARAM(x,y,z) "FILE: %s LINE: %d\nERROR: in [%s], parameter [%s] is [%s]\n", PATH_2_NAME(__FILE__), __LINE__, x, y, z
#define FAILE_NEW(x) "FILE: %s LINE: %d\nERROR: failed to new [%s] object\n", PATH_2_NAME(__FILE__), __LINE__, x
#define FAILE_DUPSTR(x) "FILE: %s LINE: %d\nERROR: failed to duplicate [%s] string\n", PATH_2_NAME(__FILE__), __LINE__, x

//-----------------------------------------------------------------------------
// 统计字符串前导空白字符数
// Parameter
//    [in] line : 被统计的字符串
// ReturnValue
//    返回空白字符数
//-----------------------------------------------------------------------------
inline var_u8 len_start_space(const var_1* line)
{
    var_u8 len = 0;
    const char* pline = line;

    while (*pline != '\0')
    {
        if (isascii(*pline) && isspace(*pline))
            pline += 1;         // ASCII空白字符（空格、TAB、换行符等）
        else if (*(var_u8*)pline == 0xA1 && *(var_u8*)(pline+1) == 0xA1)
            pline += 2;         // 中文全角空格0xA1A1
        else
            break;
    }

    return (var_u8)(pline - line);
}

//-----------------------------------------------------------------------------
// 判断字符串是否存在乱码（主要针对汉字）
// Parameter
//    [in] p : 被判断的字符串
// ReturnValue
//    返回字符串中是否存在乱码
// Remark
//    这里汉字编码指GBK/2: GB2312 汉字，并且对“阿·阿尔卡赫塔尼”中的“·”0xA1A4
//    作特别处理，并不将它被识别为乱码。
//-----------------------------------------------------------------------------
static bool badcode(const char* p)
{
    while (*p != '\0')
    {
        if (*p > 0)            // 英文字符
            p++;
        else
        {
            if (*(p+1) == '\0')
                return true;   // *p为半个汉字，*(p+1)为字符串结束符
            if (*(p+1) > 0)
                return true;   // *p为半个汉字，*(p+1)为一个英文字符
            else if (!((*(unsigned char*)p >= 0xB0 && *(unsigned char*)p <= 0xF7
                && *(unsigned char*)(p+1) >= 0xA1 && *(unsigned char*)(p+1) <= 0xFE)
                || (*(unsigned char*)p == 0xA1 && *(unsigned char*)(p+1) == 0xA4)))
                return true;   // 不能组成一个合法的汉字——乱码
            else
                p += 2;        // 合法汉字，继续检查
        }
    }

    return false;
}

// template<typename T>
// inline var_vd _swap(T& lhs, T& rhs)
// {
//     T temp = lhs;
//     lhs = rhs;
//     rhs = temp;
// }
// 
// template<typename T>
// var_vd _quick_sort(T* _term_infos, var_4 _low, var_4 _high, bool _cmp(const T&, const T&))
// {
//     assert(NULL != _term_infos);
//     if (_low >= _high)
//     {// 长度为1或...
//         return;
//     }
// 
//     var_4 mid = _low + ((_high - _low ) >> 1);
//     if (_low <= (_high - 2))
//     {// 长度不小于3
//         if (_cmp(_term_infos[_low], _term_infos[mid]))
//         {
//             _swap(_term_infos[_low], _term_infos[mid]);
//         }
//         if (_cmp(_term_infos[_low], _term_infos[_high]))
//         {
//             _swap(_term_infos[_low], _term_infos[_high]);
//         }
//         if (_cmp(_term_infos[mid], _term_infos[_high]))
//         {
//             _swap(_term_infos[mid], _term_infos[_high]);
//         }
//         if (_low == (_high - 2))
//         {// 长度为3
//             return;
//         }
//     }
//     else
//     {// 长度为2
//         if (_cmp(_term_infos[_low], _term_infos[_high]))
//         {
//             _swap(_term_infos[_low], _term_infos[_high]);
//         }
//         return;
//     }
//     T pivot = _term_infos[mid];
//     var_4 i = _low, j = _high;
//     while (i <= j)
//     {
//         while (_cmp(_term_infos[j], pivot))
//         {
//             --j;
//         }
//         while (_cmp(pivot, _term_infos[i]))
//         {
//             ++i;
//         }
//         if (i <= j)
//         {
//             _swap(_term_infos[i], _term_infos[j]);
//             --j;
//             ++i;
//         }
//     }
// 
//     if (_low < j)
//     {
//         _quick_sort(_term_infos, _low, j, _cmp);
//     }
//     if (_high > i)
//     {
//         _quick_sort(_term_infos, i, _high, _cmp);
//     }
// }

inline var_4 assign_word_type(var_4& _WT, var_1* _tag)
{
    if ('n' == *_tag)
    {
        SET_FLAG(_WT, WT_NOUN);
        if (!strcmp("nr", _tag))
        {
            SET_FLAG(_WT, WT_NAME);
        }
    }
    else if ('v' == *_tag)
    {
        SET_FLAG(_WT, WT_VERB);
    }
    else
    {
        SET_FLAG(_WT, WT_OTHR);
    }

    return RET_SECCEED;
}

#endif


