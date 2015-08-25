#ifndef __BASE_WORD_EXTRACTOR_H_
#define __BASE_WORD_EXTRACTOR_H_

#include "NLPIR.h"
#include "UC_ReadConfigFile.h"
#include "ext_common_def.h"
#include "UC_MD5.h"
#include "UT_HashSearch.h"

typedef struct tag_base_conf
{
    var_1 _self_cut : 1;        //0-> 文本未切词，需系统自己切词
    
    var_1 _trunc_tit : 1;       //1-> 标题需截取
    var_1 _tit_max_len : 6;     // 标题截取最大长度
    var_u1 _tit_weight;         // // 标题词在正文中出现时，所加的权重[0~255]（5代表相当于在正文中出现5次）

    var_4 _trunc_text : 1;      //1-> 正文需截取
    var_4 _text_max_len : 31;    // 正文截取最大长度

    //var_u2 _feature_min_num;    //最少特征词个数，如文本过短则提取失败 （常常是英文新闻、乱码新闻、无正文、短新闻）
    var_u2 _feature_max_num;
    
    var_1 _zky_data[256];
}base_conf, *pbase_conf;

class UC_ReadConfigFile;

class base_word_extractor
{
public:
    base_word_extractor();

    virtual ~base_word_extractor();

    //FUNC  初始化
    //IN    _cfg_path: 配置文件路径
    //RET   0：正常 其他：错误码 可用get_error_description获取错误信息
	virtual var_4 init(var_1* _cfg_path, var_vd* _share_pointer = NULL) = 0;

    //FUNC  反初始化
    //RET   空
    virtual var_vd uninit() = 0;

    //FUNC  执行抽取
    //IN    _doc_pointer: 待抽取的文本指针 资讯格式：docid(8)+titlen(4)+title(titlen)+doclen(4)+doc(doclen)
    //IN    _doc_len: 待抽取的文本长度       
    //RET   0：正常 其他：错误码 可用get_error_description获取错误信息
    virtual var_4 extract(var_u4 _doc_len, var_1* _doc_pointer) = 0;
	
    //FUNC  获取词语信息
    //IN    _calc_type: 待获取的词语类型
    //OUT   _term_pointer: 待写入词语信息缓冲区
    //IN    _term_cnt: 待写入的最大词语信息个数
    //RET   0：正常 其他：错误码 可用get_error_description获取错误信息
    virtual var_4 get_term(word_type_e _type, term_info_st* _term_pointer, var_u4 _term_cnt) = 0;
	
    //FUNC  获取词语个数
    //IN    _calc_type: 待获取的词语类型
    //RET   0：正常 其他：错误码 可用get_error_description获取错误信息
    virtual var_4 get_count(word_type_e _type) = 0;

    //FUNC  获取错误信息
    //IN    _err_code: 错误码
    //RET   0：正常 其他：获取到的错误信息
    virtual var_1* get_error_description(var_4 _err_code) = 0;

protected:
    var_4 parse_news_buf(
        var_1* _news_buf, 
        var_u4 _news_len
        );

    var_4 _init(
        UC_ReadConfigFile* _reader
        );

    var_vd _uninit();

    var_vd normalize_vector(
        term_info_st* _features, 
        var_u4 _cnt
        );

private:
    var_4 read_conf(
        UC_ReadConfigFile* _reader
        );

protected:
    var_u4          m_status;
    base_conf       m_base_conf;
    simple_pair<var_1*, var_1*> m_tit_inf;
    simple_pair<var_1*, var_1*> m_text_inf;
    var_u8          m_news_ID;

    UC_MD5 m_md5er;

    //存储提取出的特征词信息<var_u8, feature*>
    UT_HashSearch<var_u8> m_feature_map;
};

#endif

