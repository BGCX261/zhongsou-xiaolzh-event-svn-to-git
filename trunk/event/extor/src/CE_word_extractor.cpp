#include "CE_word_extractor.h"

CE_word_extractor::CE_word_extractor()
{

}

CE_word_extractor::~CE_word_extractor()
{

}

//FUNC  初始化
//IN    _cfg_path: 配置文件路径
//RET   0：正常 其他：错误码 可用get_error_description获取错误信息
var_4 CE_word_extractor::init(
    var_1* _cfg_path,
    var_vd* _share_pointer /*= NULL*/)
{
    m_status = 0;    

    UC_ReadConfigFile cur_reader;
    var_4 ret = cur_reader.InitConfigFile(_cfg_path);
    if (ret)
    {
        LOG_FAILE_CALL_PARAM("TF_word_extractor::init", "cur_reader.InitConfigFile", _cfg_path);
        return RET_ERROR_INVALID_PARAM;
    }
        
    ret = base_word_extractor::_init(&cur_reader);
    if (ret)
    {
        LOG_FAILE_CALL_PARAM("TF_word_extractor::init", "base_word_extractor::read_conf", _cfg_path);
        return RET_ERROR_INVALID_PARAM;        
    }

    SET_FLAG(m_status, WE_init);

    return 0;
}

//FUNC  执行抽取
//IN    _doc_pointer: 待抽取的文本指针
//IN    _doc_len: 待抽取的文本长度
//RET   0：正常 其他：错误码 可用get_error_description获取错误信息
var_4 CE_word_extractor::extract(
    var_u4 _doc_len, 
    var_1* _doc_pointer)
{
    if (!TEST_FLAG(m_status, WE_init))
    {
        LOG_ERROR("请先调用", "TF_word_extractor::init");
        return RET_ERROR_INVALID_PARAM;    
    }
    RESET_FLAG(m_status, WE_EXTR);

    var_4 ret = base_word_extractor::parse_news_buf(_doc_pointer, _doc_len);
    if (ret)
    {
        LOG_FAILE_CALL_RET("graph_word_extractor::extract", "base_word_extractor::parse_news_buf", ret);
        return RET_ERROR_INVALID_PARAM;            
    }

    {
        ///////////////////////////抽取代码//////////////////////////////////////
    }

    SET_FLAG(m_status, WE_EXTR);

    return 0;
}

//FUNC  获取词语信息
//IN    _calc_type: 待获取的词语类型
//OUT   _term_pointer: 待写入词语信息缓冲区
//IN    _term_cnt: 待写入的最大词语信息个数
//RET   0：正常 其他：错误码 可用get_error_description获取错误信息
var_4 CE_word_extractor::get_term(
    word_type_e _type, 
    term_info_st* _term_pointer, 
    var_u4 _term_cnt)
{
    return 0;
}

//FUNC  获取词语个数
//IN    _calc_type: 待获取的词语类型
//RET   0：正常 其他：错误码 可用get_error_description获取错误信息
var_4 CE_word_extractor::get_count(
    word_type_e _type)
{
    return 0;
}

//FUNC  获取错误信息
//IN    _err_code: 错误码
//RET   0：正常 其他：获取到的错误信息
var_1* CE_word_extractor::get_error_description(
    var_4 _err_code)
{
    return 0;
}

var_vd CE_word_extractor::uninit()
{
    base_word_extractor::_uninit();
}

