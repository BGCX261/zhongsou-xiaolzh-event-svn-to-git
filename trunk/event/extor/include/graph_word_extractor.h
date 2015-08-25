#ifndef __GRAPH_WORD_EXTRACTOR_H_
#define __GRAPH_WORD_EXTRACTOR_H_

#include "NLPIR.h"
#include "UC_ReadConfigFile.h"
#include "base_word_extractor.h"

class graph_word_extractor : public base_word_extractor
{
public:
    graph_word_extractor();

	virtual ~graph_word_extractor();

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

};

#endif

