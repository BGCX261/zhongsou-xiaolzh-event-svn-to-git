#ifndef __WORD_IDF_CALCOR_H_
#define __WORD_IDF_CALCOR_H_

#include "ext_common_def.h"
#include "UT_HashTable_Pro.h"
#include "UC_ReadConfigFile.h"

class word_IDF_calcor
{
public:
    word_IDF_calcor();

	~word_IDF_calcor();

    //FUNC  初始化
    //IN    _cfg_path: 配置文件路径
    //RET   0：正常 其他：错误码
	var_4 init(
        var_1* _cfg_path
        );

    //FUNC  反初始化
    //RET   空
    var_vd uninit();

    var_4 inc_totalDF()
    { 
        ++m_total_DF;
        return m_total_DF;
    }

    var_4 update(
        term_info_st** _words, 
        var_u4 _word_cnt
        );

    var_f4 get_IDF(
        var_u8 _word_id
        );
    
private:
    var_4 m_status;

    var_u4  m_total_DF;

    //存储词语DF信息<var_u8, var_u4>
    UT_HashTable_Pro<var_u8>  m_DF_hash;

    var_1   m_TF_data[256];
};

#endif

