#include "zky_word_extractor.h"

zky_word_extractor::zky_word_extractor()
    :base_word_extractor()
{
    m_status = 0;    
}

zky_word_extractor::~zky_word_extractor()
{

}

//FUNC  初始化
//IN    _cfg_path: 配置文件路径
//RET   0：正常 其他：错误码 可用get_error_description获取错误信息
var_4 zky_word_extractor::init(
    var_1* _cfg_path,
    var_vd* _share_pointer /*= NULL*/)
{
    if (TEST_FLAG(m_status, WE_init))
    {
        LOG_ERROR("此对象", "已经初始化完成");
        return RET_ERROR_INVALID_PARAM; 
    }
    
    if (!_cfg_path)
    {
        LOG_INVALID_PARAMETER("_cfg_path", "NULL");
        return RET_ERROR_INVALID_PARAM;
    }

    UC_ReadConfigFile cur_reader;
    var_4 ret = cur_reader.InitConfigFile(_cfg_path);
    if (ret)
    {
        LOG_FAILE_CALL_PARAM("zky_word_extractor::init", "cur_reader.InitConfigFile", _cfg_path);
        return RET_ERROR_INVALID_PARAM;
    }
        
    ret = base_word_extractor::_init(&cur_reader);
    if (ret)
    {
        LOG_FAILE_CALL_PARAM("zky_word_extractor::init", "base_word_extractor::read_conf", _cfg_path);
        return RET_ERROR_INVALID_PARAM;        
    }

    if (!m_base_conf._self_cut)
    {
        LOG_ERROR("zky_word_extractor::init", "m_base_conf._self_cut");
        return RET_ERROR_INVALID_PARAM;  
    }
    
    m_buf_size = 4<<20;
    m_buf_ptr = new var_1[m_buf_size];
    if (!m_buf_ptr)
    {
        LOG_FAILE_NEW("m_buf_ptr");
        return RET_NO_ENOUGH_MEMORY;
    }

    m_features_size = 0;
    m_features_capacity = MAX_NEWS_FEATURE_NUM;
    m_features_pointer = new term_info_st[m_features_capacity];
    if (!m_features_pointer)
    {
        LOG_FAILE_NEW("m_terms_pointer");
        return RET_NO_ENOUGH_MEMORY;
    }
 
    SET_FLAG(m_status, WE_init);

    return 0;
}

//FUNC  执行抽取
//IN    _doc_pointer: 待抽取的文本指针
//IN    _doc_len: 待抽取的文本长度
//RET   0：正常 其他：错误码 可用get_error_description获取错误信息
var_4 zky_word_extractor::extract(
    var_u4 _doc_len, 
    var_1* _doc_pointer)
{
    if (!TEST_FLAG(m_status, WE_init))
    {
        LOG_ERROR("请先调用", "zky_word_extractor::init");
        return RET_ERROR_INVALID_PARAM;    
    }
    RESET_FLAG(m_status, WE_EXTR);

    var_4 ret = base_word_extractor::parse_news_buf(_doc_pointer, _doc_len);
    if (ret)
    {
        LOG_FAILE_CALL_RET("zky_word_extractor::extract", "base_word_extractor::parse_news_buf", ret);
        return RET_ERROR_INVALID_PARAM;            
    }

    var_1* pos = m_buf_ptr;
    var_u4 remain = m_buf_size;
    if (remain > (m_tit_inf.right - m_tit_inf.left))
    {
        memcpy(pos, m_tit_inf.left, m_tit_inf.right - m_tit_inf.left);
        pos += m_tit_inf.right - m_tit_inf.left;
        remain -= m_tit_inf.right - m_tit_inf.left;
    }
    else
    {
        memcpy(pos, m_tit_inf.left, m_tit_inf.right - m_tit_inf.left);
        pos += remain;
        remain -= remain;
    }

    if (remain)
    {
        *pos++ = ' ';
        if (remain > (m_text_inf.right - m_text_inf.left))
        {
            memcpy(pos, m_text_inf.left, m_text_inf.right - m_text_inf.left);
            pos += m_text_inf.right - m_text_inf.left;
            remain -= m_text_inf.right - m_text_inf.left;
        }
        else
        {
            memcpy(pos, m_text_inf.left, m_text_inf.right - m_text_inf.left);
            pos += remain;
            remain -= remain;
        }
    }
    
    ret = calc_weight(m_buf_ptr, pos - m_buf_ptr);
    if (ret)
    {
        LOG_FAILE_CALL_RET("zky_word_extractor::extract", "zky_word_extractor::calc_weight", ret);
        return RET_ERROR_INVALID_PARAM;                    
    }

    SET_FLAG(m_status, WE_EXTR);

    return RET_SECCEED;
}

//FUNC  获取词语信息
//IN    _calc_type: 待获取的词语类型
//OUT   _term_pointer: 待写入词语信息缓冲区
//IN    _term_cnt: 待写入的最大词语信息个数
//RET   0：正常 其他：错误码 可用get_error_description获取错误信息
var_4 zky_word_extractor::get_term(
    word_type_e _type, 
    term_info_st* _term_pointer, 
    var_u4 _term_cnt)
{
    term_info_st* pos_wt = _term_pointer;
    term_info_st* end_wt = _term_pointer + _term_cnt;

    term_info_st* pos = m_features_pointer;
    term_info_st* end = m_features_pointer + m_features_size;

    for (; end > pos && end_wt > pos_wt; ++pos)
    {
        if ((_type & pos->type) || (_type == WT_ALL))
        {
            *pos_wt++ = *pos;
        }  
    }

    return pos_wt - _term_pointer;
}

//FUNC  获取词语个数
//IN    _calc_type: 待获取的词语类型
//RET   0：正常 其他：错误码 可用get_error_description获取错误信息
var_4 zky_word_extractor::get_count(
    word_type_e _type)
{
    if (TEST_FLAG(_type, WT_ALL))
    {
        return m_features_size;    
    }

    var_4 ret = 0;
    term_info_st* pos = m_features_pointer;
    term_info_st* end = m_features_pointer + m_features_size;
    for (; end > pos; ++pos)
    {
        if ((_type & pos->type) || (_type == WT_ALL))
        {
            ++ret;
        }  
    }

    return ret;
}

//FUNC  获取错误信息
//IN    _err_code: 错误码
//RET   0：正常 其他：获取到的错误信息
var_1* zky_word_extractor::get_error_description(
    var_4 _err_code)
{
    return 0;
}

var_vd zky_word_extractor::uninit()
{
    base_word_extractor::_uninit();
}

var_4 zky_word_extractor::calc_weight(var_1* _doc_beg, var_u4 _doc_len)
{
    assert(_doc_beg);
    if (0 >= _doc_len)
    {
        LOG_INVALID_PARAMETER("_doc_len", "0");
        return RET_ERROR_INVALID_PARAM;    
    }
    _doc_beg[_doc_len] = '\0';

    var_1* pos = (var_1*)NLPIR_GetKeyWords(_doc_beg, m_base_conf._feature_max_num, true);
    if (!pos || !(*pos))
    {
        LOG_FAILE_CALL("zky_word_extractor::calc_weight", "::NLPIR_GetKeyWords");
        return RET_ERROR_INVALID_PARAM;
    }

    var_1* word_beg = NULL;
    var_1* word_end = NULL;
    var_1* tag_beg = NULL;
    var_1* tag_end = NULL;
    var_1* wi_beg = NULL;
    var_1* wi_end = NULL;

    m_features_size = 0;

    var_4 space_len = 0;
    for (; *pos;)
    {
        //过滤前导空白
        while (*pos)
        {
            space_len = len_start_space(pos);
            if (0 < space_len)
            {
                pos += space_len;
            }
            else
            {
                break;
            }
        }
        word_beg = pos;

        //识别词语
        while (*pos && ' ' != *pos)
        {
            ++pos;
        }
        wi_end = pos;
        if (!(*pos))
        {
            break;
        }
        ++pos;
        
        //识别词性
        word_end = word_beg;
        for (; '/' != *word_end && word_end < wi_end; ++word_end)
        {
            if (0 > *word_end)
            {
                ++word_end;
            }
        }
        tag_beg = word_end + 1;

        //识别词性
        tag_end = tag_beg;
        for (; '/' != *tag_end && tag_end < wi_end; ++tag_end)
        {
            if (0 > *tag_end)
            {
                ++tag_end;
            }
        }
        wi_beg = tag_end + 1;
        
        term_info_st* cur_feature = m_features_pointer + m_features_size;

        *wi_end = '\0';
        cur_feature->weight = atof(wi_beg);
        *wi_end = ' ';

        *tag_end = '\0';
        cur_feature->type = 0;
        assign_word_type(cur_feature->type, tag_beg);
        *tag_end = '/';

        cur_feature->word_id = m_md5er.MD5Bits64((var_u1*)word_beg, word_end - word_beg);
        memcpy(cur_feature->word_str, word_beg, word_end - word_beg);
        cur_feature->word_str[word_end - word_beg] = '\0';
        cur_feature->tf = 0;
        cur_feature->idf = 0;
        ++m_features_size;

        if (m_features_size == m_features_capacity + 1)
        {
            LOG_ERROR("文本过长", "特征词缓冲区不够用！！！");
            break;
        }
    }

    return RET_SECCEED;
}
