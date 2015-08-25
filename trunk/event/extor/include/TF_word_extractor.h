#ifndef __TF_WORD_EXTRACTOR_H_
#define __TF_WORD_EXTRACTOR_H_

#include "NLPIR.h"
#include "ext_common_def.h"
#include "UT_Allocator.h"
#include "UT_HashSearch.h"
#include "UT_HashTable_Pro.h"
#include "word_IDF_calcor.h"
#include "base_share_container.h"
#include "UC_ReadConfigFile.h"
#include "base_word_extractor.h"

class base_share_container;

class TF_word_extractor : public base_word_extractor
{
public:
    TF_word_extractor();

	virtual ~TF_word_extractor();

    //FUNC  初始化
    //IN    _cfg_path: 配置文件路径
    //RET   0：正常 其他：错误码 可用get_error_description获取错误信息
	var_4 init(
        var_1* _cfg_path,
        var_vd* _share_pointer = NULL
        );

    //FUNC  反初始化
    //RET   空
    virtual var_vd uninit();
	
    //FUNC  执行抽取
    //IN    _doc_pointer: 待抽取的文本指针
    //IN    _doc_len: 待抽取的文本长度
    //RET   0：正常 其他：错误码 可用get_error_description获取错误信息
    var_4 extract(
        var_u4 _doc_len, 
        var_1* _doc_pointer
        );
	
    //FUNC  获取词语信息
    //IN    _calc_type: 待获取的词语类型
    //OUT   _term_pointer: 待写入词语信息缓冲区
    //IN    _term_cnt: 待写入的最大词语信息个数
    //RET   0：正常 其他：错误码 可用get_error_description获取错误信息
    var_4 get_term(
        word_type_e _type, 
        term_info_st* _term_pointer, 
        var_u4 _term_cnt
        );
	
    //FUNC  获取词语个数
    //IN    _calc_type: 待获取的词语类型
    //RET   0：正常 其他：错误码 可用get_error_description获取错误信息
    var_4 get_count(
        word_type_e _type
        );

    //FUNC  获取错误信息
    //IN    _err_code: 错误码
    //RET   0：正常 其他：获取到的错误信息
    var_1* get_error_description(
        var_4 _err_code
        );

private:
    var_4 calc_weight(
        var_1* _doc_beg, 
        var_1* _doc_end, 
        var_u2 _factor
        );

    var_4 select_feature();

    var_4 need_consider(
        var_1* _word
        );

private:
    term_info_st*    m_features_pointer;
    var_u4      m_features_capacity;
    var_u4      m_features_size;
    base_share_container* m_share_container;

    term_info_st**   m_sel_features;
    var_u4      m_sel_cnt;
};

#endif

