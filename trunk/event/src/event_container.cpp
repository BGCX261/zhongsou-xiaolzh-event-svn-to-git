#include "event_container.h"

event_container::event_container()
: m_evt_infer()
, m_doc_infer()
, m_evt_allocator(NULL)
, m_evt_size(0u)
, m_dnode_allocator(NULL)
, m_dnode_size(0u)
, m_enode_allocator(NULL)
, m_enode_size(0u)
, m_pointer_allocator(NULL)
, m_pointer_size(0u)
, m_evt_ptr(NULL)
{
    
}
	
event_container::~event_container()
{

}

var_4 event_container::add_doc(doc_infer* _dinfer)
{
	var_4 ret;
	doc_infer* dinfer = NULL;
    event_infer* einfer = NULL;
    m_lck.lock_w();
    try
    {   //1. analysis
        ret = m_doc_indexer.SearchKey_FL(_dinfer->m_id, (var_vd**)&dinfer);
        if (ret)
        {
			// if doc exist, update it's information 
            if (!dinfer)
            {
                throw RET_ERROR_INVALID_PARAM;
            }
			_dinfer->m_eid = dinfer->m_eid;	
			// update event_infer
	        ret = m_evt_indexer.SearchKey_FL(dinfer->m_eid, (var_vd**)&einfer);
            if (ret)
            {
                LOG_FAILE_CALL_RET("event_container::add_doc", "m_evt_indexer.SearchKey_FL", ret);
                throw RET_ERROR_INVALID_PARAM;
            }

            ret = einfer->update(_dinfer, 1);
            if (ret)
            {
                LOG_FAILE_CALL_RET("event_container::add_doc", "einfer->update", ret);
                throw RET_ERROR_INVALID_PARAM;
            }

            throw RET_FALSE;
        }
		//!!! if doc not exist ,add dinfer to m_doc_indexer and m_doc_infer
		// 正排
		if (m_doc_infer.m_size == m_doc_infer.m_capacity)
		{
			//delete old docs
				
		}
		doc_infer& tail = m_doc_infer.m_pointer[m_doc_infer.m_size];
		tail.copy(_dinfer);
		m_doc_infer.m_size++;
		// 倒排
		ret = m_doc_indexer.AddKey_FL(_dinfer->m_id, (var_vd*)&tail, NULL);
		if (ret < 0)
		{
			LOG_FAILE_CALL_RET("event_container::add_doc", "m_doc_indexer->AddKey_FL", ret);
			throw RET_FALSE;
		}
		//
        m_stat_hash.ClearHashSearch();
        var_u4 idx = 0u; 
        term_info_st*& cur_terms = _dinfer->m_terms.m_pointer;
        whl_list<doc_infer* >* did_sets = NULL;
        var_vd* pre = NULL;
        const var_4 one = 1;
        for (; _dinfer->m_terms.m_size > idx; ++idx)
        {
            ret = m_w2doc_map.SearchKey_FL(cur_terms->word_id, (var_vd**)&did_sets);
            if (ret)
            {
                continue;
            }
            
            whl_node<doc_infer*>* itr = did_sets->m_head;
            for (; itr; itr = itr->_right)
            {
                ret = m_stat_hash.AddKey_FL((var_p)itr->_val, (var_vd*)&one, &pre);
                if (1 == ret)
                {
                    assert(pre);
                    ++(*(var_u4*)pre);
                }
                else if (ret)
                {
                    assert(0);
                }
                else
                {
                    ;
                }
            }
        }

        m_stat_hash.PreTravelKey();
        var_vd* cnt = NULL;
        var_4 len = 0u;
        var_vd** tmp = 0;
        var_p id = 0;
        simple_pair<doc_infer*, var_f4> max_matcher;
        max_matcher.right = -1.f;
        max_matcher.left = NULL;

        m_cdd_docers.reset();
        simple_pair<doc_infer*, var_f4> matcher;
        for (; ;)
        {
            ret = m_stat_hash.TravelKey(id, cnt, len);
            if (ret)
            {
                break;
            }

//             if (KW_MIN_NUM > *(var_u4*)cnt)
//             {
//                 continue;
//             }

            var_f4 dist = measure_docdist(_dinfer, (doc_infer*)(id));
            if (0.f > dist || MAX_MAX_THRESHOLD < dist)
            {
                LOG_FAILE_CALL_ID("event_container::add_doc", "event_container::measure_docinfs", id);
                continue;                
            }

            matcher.left = (doc_infer*)(id);
            matcher.right = dist;
            if (max_matcher.right > dist)
            {
                max_matcher = matcher;
            }

            ret = m_cdd_docers.add(matcher);
            if (ret)
            {
                assert(0);
            }
        }

        if (max_matcher.right < MIN_MIN_THRESHOLD)
        {// 极其相近，归属于该文档所在事件集
            ret = doc_into_event(_dinfer, max_matcher.left->m_eid);
            if (ret)
            {
                LOG_FAILE_CALL_ID("event_container::add_doc", "event_container::doc_into_event", id);
                throw RET_ERROR_INVALID_PARAM;
            }

            throw RET_SECCEED;
        }

        whl_node<simple_pair<doc_infer*, var_f4> >* itr = m_cdd_docers.m_head->_right;
        if (!itr)
        {// just one similar doc
            ret = create_new_event(_dinfer);
            if (ret)
            {
                LOG_FAILE_CALL_RET("event_container::add_doc", "event_container::create_new_event", ret);
                throw RET_ERROR_INVALID_PARAM;
            }
            
            throw RET_SECCEED;
        }

        idx = 1u;
        var_u8 pre_eid = m_cdd_docers.m_head->_val.left->m_eid;
        var_u4 doc_cnt = 1;
        whl_node<simple_pair<doc_infer*, var_f4> >* head = itr;
        m_cdd_evters.reset();
		
		doc_evt_dst evt_score;
		var_f4 max_sm = -1.f;
		// 统计每个事件的支持票数， 取前两个做候选事件
		// perhaps the loop is
		// for (; m_cdd_docers.m_cnt >= idx; ++idx, itr = itr->_right)
		// {
		// 		if (itr == NULL || pre_eid != itr->_val.left->m_eid)
		// 		{
		//			// do something
		// 		}
		// }
		for (; m_cdd_docers.m_cnt > idx; ++idx, itr = itr->_right)
        {
            if (pre_eid != itr->_val.left->m_eid)
            {
               	evt_score._score = calc_event_score(head, itr, _dinfer);
				if (0.f > evt_score._score)
                {
                    LOG_FAILE_CALL_RET("event_container::add_doc", "event_container::calc_event_score", (var_4)(evt_score._score * 100));
                    throw RET_ERROR_INVALID_PARAM;
                }
                
	            ret = m_evt_indexer.SearchKey_FL(pre_eid, (var_vd**)&evt_score._einfer);
                if (ret)
                {
                    LOG_FAILE_CALL_RET("event_container::add_doc", "m_evt_indexer.SearchKey_FL", ret);
                    throw RET_ERROR_INVALID_PARAM;
                }

                evt_score._doc_cnt = doc_cnt;
                evt_score._max_doc_score = max_sm;

                ret = m_cdd_evters.add(evt_score);
                if (ret)
                {
                    assert(0);
                }

                doc_cnt = 1;
                max_sm = -1.f;
                head = itr;
                pre_eid = itr->_val.left->m_eid;
            }

            max_sm = max(max_sm, itr->_val.right);
            ++doc_cnt;
        }

		//1. 计算票数最高者的事件归属度
		//2.1 若满足条件 则归属之
		//2.2 不满足 则创建新事件
		//2.1.1 合并事件

        {// last chance !!!
            evt_score._score = calc_event_score(head, itr, _dinfer);
            if (0.f > evt_score._score )
            {
                LOG_FAILE_CALL_RET("event_container::add_doc", "event_container::calc_event_score", (var_4)(evt_score._score * 100));
                throw RET_ERROR_INVALID_PARAM;
            }
                
            ret = m_evt_indexer.SearchKey_FL(pre_eid, (var_vd**)&evt_score._einfer);
            if (ret)
            {
                LOG_FAILE_CALL_RET("event_container::add_doc", "m_evt_indexer.SearchKey_FL", ret);
                throw RET_ERROR_INVALID_PARAM;
            }

            evt_score._doc_cnt = doc_cnt;
            evt_score._max_doc_score = max_sm;

            ret = m_cdd_evters.add(evt_score);
            if (ret)
            {
                assert(0);
            }
        }
       
        assert(m_cdd_evters.m_head);
		if ((m_cdd_evters.m_head->_val._doc_cnt<<1 > m_cdd_evters.m_head->_val._einfer->m_doc_cnt) ||
            (m_cdd_evters.m_head->_val._max_doc_score < MIN_MIN_THRESHOLD))
        {// 1.属于事件ei的相似新闻个数超过事件ei的种子个数的1/2
         // 2.最大的新闻相似值max_nscore大于highThreshold

            ret = doc_into_event(_dinfer, m_cdd_evters.m_head->_val._einfer->m_id);
            if (ret)
            {
                LOG_FAILE_CALL_ID("event_container::add_doc", "event_container::doc_2_event", id);
                throw RET_ERROR_INVALID_PARAM;
            }

            if (!m_cdd_evters.m_head->_right)
            {// 无事件2
                throw RET_SECCEED;
            }

            ret = adjust_2_events(
                m_cdd_evters.m_head->_val._einfer->m_id,
                m_cdd_evters.m_head->_right->_val._einfer->m_id);
            if (ret)
            {
                LOG_FAILE_CALL_ID("event_container::add_doc", "event_container::doc_2_event", id);
                throw RET_ERROR_INVALID_PARAM;
            }
        }
        else
        {
            ret = create_new_event(_dinfer);
            if (ret)
            {
                LOG_FAILE_CALL_RET("event_container::add_doc", "event_container::create_new_event", ret);
                throw RET_ERROR_INVALID_PARAM;
            }
        }

        throw RET_SECCEED;
    }
    catch (const var_4 _err_code)
    {
		// 更新核心词表
		ret = update_key_list(_dinfer);
		if (ret)
		{
			LOG_FAILE_CALL_RET("event_container::add_doc", "event_container::update_key_list", ret);
		}
        m_lck.unlock();
        return _err_code;	
    }
}

var_4 event_container::update_key_list(doc_infer* dinfer)
{
	var_u4 idx = 0, ret = 0;
	var_u8 word_id = 0;
	try
	{	// update m_w2doc_map
		whl_list<doc_infer* >* did_sets = NULL;
		for (idx = 0; idx < dinfer->m_terms.m_size; ++idx)
		{
			word_id = dinfer->m_terms.m_pointer[idx].word_id;
			ret = m_w2doc_map.SearchKey_FL(word_id, (var_vd**)&did_sets);
			if (ret) 
			{// ret == -1, word_id not exists
				var_4 key_doc_num = 4<<20;	
				whl_list<doc_infer*> tmp_sets;
				ret = tmp_sets.init(NULL, NULL, key_doc_num);
				if (ret)
				{
					LOG_FAILE_CALL_RET("event_container::update_key_list", "tmp_sets.init", ret);
					throw RET_FALSE; 
				}
				ret = tmp_sets.add(dinfer);
				if (ret)
				{
					LOG_FAILE_CALL_RET("event_container::update_key_list", "tmp_set.add", ret);
					throw RET_FALSE;
				}
				ret = m_w2doc_map.AddKey_FL(word_id, (var_vd*)&tmp_sets, NULL);
				if (ret < 0)
				{
					LOG_FAILE_CALL_RET("event_container::update_key_list", "m_w2doc_map.AddKey_FL", ret);
					throw RET_FALSE;
				}
			}
			else
			{// ret == 0, word_id exists 		
				ret = did_sets->add(dinfer);
				if (ret)
				{
					LOG_FAILE_CALL_RET("event_container::update_key_list", "did_sets->add[dinfer already exists]", ret);
					// need not exit
				}
			}
		}
		
		// update m_w2evt_map
		event_infer* _einfer = NULL;
		ret = m_evt_indexer.SearchKey_FL(dinfer->m_eid, (var_vd**)&_einfer);
		if (ret)
		{
			LOG_FAILE_CALL_RET("event_container::update_key_list", "m_evt_indexer.SearchKey_FL", ret);
			throw RET_FALSE;
		}
		whl_list<event_infer* >* evt_sets = NULL;
		for (idx = 0; idx < dinfer->m_terms.m_size; ++idx)
		{
			word_id = dinfer->m_terms.m_pointer[idx].word_id;
			ret = m_w2evt_map.SearchKey_FL(word_id, (var_vd**)&evt_sets);
			if (ret)
			{//ret == -1, word_id not exists
				var_4 key_evt_num = 4<<20;
				whl_list<event_infer* > tmp_sets;
				ret = tmp_sets.init(NULL, NULL, key_evt_num);
				if (ret)
				{
					LOG_FAILE_CALL_RET("event_container::update_key_list", "tmp_sets.init", ret);
					throw RET_FALSE;
				}
				ret = tmp_sets.add(_einfer);
				if (ret)
				{
					LOG_FAILE_CALL_RET("event_container::update_key_list", "tmp_sets.add", ret);
				}
				ret = m_w2evt_map.AddKey_FL(word_id, (var_vd*)&tmp_sets, NULL);
				if (ret < 0)
				{
					LOG_FAILE_CALL_RET("event_container::update_key_list", "m_w2doc_map.AddKey_FL", ret);
					throw RET_FALSE;
				}
			}
			else
			{// ret ==0, word_id exists
				ret = evt_sets->add(_einfer);
				if (ret)
				{
					LOG_FAILE_CALL_RET("event_container::update_key_list", "evt_sets->add[einfer already exists]", ret);
					// need not exit
				}
			}
		}

		// boult out of date docs and events
		for (idx = 0; idx < dinfer->m_terms.m_size; ++idx)
		{
			boult(dinfer->m_terms.m_pointer[idx].word_id, dinfer->m_tm);
		}
	}
	catch(const var_4 _err_code)
	{
		return _err_code;
	}
}

var_4 event_container::adjust_2_events(var_u8 _leid, var_u8 _reid)
{   
    // do not consider in phase 1
	return RET_SECCEED;
}

var_4 event_container::create_new_event(doc_infer* _dinfer)
{   
    if (m_evt_infer.m_size == m_evt_infer.m_capacity)
    {
        return RET_ERROR_INVALID_PARAM;
        // del old&small event, release event memory
    }
	// 正排
    event_infer& cur_evt = m_evt_infer.m_pointer[m_evt_infer.m_size];
    cur_evt.m_id = _dinfer->m_id;
	// 倒排
    var_4 ret = m_evt_indexer.AddKey_FL(cur_evt.m_id, (var_vd*)&cur_evt, NULL);
	if (ret < 0)
	{
		LOG_FAILE_CALL_RET("event_container::create_new_event", "m_evt_indexer.AddKey_FL", ret);
		return RET_ERROR_INVALID_PARAM;
	}	
	ret = cur_evt.regist(_dinfer);
    if (ret)
    {
        return RET_ERROR_INVALID_PARAM;
    }

    ++m_evt_infer.m_size;

	return RET_SECCEED;
}

var_f4 event_container::calc_event_score(
    whl_node<simple_pair<doc_infer*, var_f4> >* begin, 
    whl_node<simple_pair<doc_infer*, var_f4> >* end, 
    doc_infer* _dinfer)
{    
    //!!!
	var_f4 _evt_score = 0;
	// knear_ns 中的doc和事件einfer中的doc求并集
	// （A并B） = （A加B） - （A交B）
	// union, intersection
	var_4  _union_doc = 0, _inter_doc = 0;
	whl_node<simple_pair<doc_infer*, var_f4> >* itr = begin;
	for (;itr != end; itr = itr->_right)
	{
		_evt_score += itr->_val.right;
		_inter_doc++;
	}
	var_u8 _evt_id = begin->_val.left->m_eid;
	event_infer *_einfer = NULL;
	var_4 ret = m_evt_indexer.SearchKey_FL(_evt_id, (var_vd**)&_einfer);
	if (ret)
	{
		LOG_FAILE_CALL_RET("event_container::calc_event_score", "m_evt_indexer.SearchKey_FL", ret);
		return RET_ERROR_INVALID_PARAM;
	}
	_union_doc = _einfer->m_did_rn_map.m_cnt + m_cdd_docers.m_cnt - _inter_doc;

	return (_evt_score / (_union_doc * 1.0));
}

var_4 event_container::doc_into_event(doc_infer* _dinfer, const var_u8 _eid)
{    
	//1. get event_infer;
	//2. regist

	event_infer* _einfer = NULL;
	var_4 ret = m_evt_indexer.SearchKey_FL(_eid, (var_vd**)&_einfer);
	if (ret)
	{
		LOG_FAILE_CALL_RET("event_container::doc_into_event", "m_evt_indexer.SearchKey_FL", ret);
		return RET_FALSE;
	}
	_einfer->regist(_dinfer);
	_dinfer->m_eid = _eid;

	return RET_SECCEED;
}

var_4 event_container::del_doc(const var_u8 _did)
{
   	var_u4 idx = 0;
	doc_infer* dinfer = NULL;
	var_4 ret = m_doc_indexer.SearchKey_FL(_did, (var_vd**)&dinfer);
	if (ret)
	{
		LOG_FAILE_CALL_RET("event_container::del_doc", "m_doc_indexer.SearchKey_FL", ret);
		return RET_FALSE;	
	}

	//1. del doc from doc storage
	ret = m_doc_storager->del(_did);
	if (ret)
	{
		LOG_FAILE_CALL_RET("event_container::del_doc", "m_doc_storager.del", ret);
		return RET_FALSE;
	}

	//2. del doc information
   	m_w2doc_map.PreTravelKey();
	whl_list<doc_infer*> *did_sets = NULL;
	var_vd *temp_ptr = NULL;
	var_u8 word_id = 0;
	var_4 element_size = 0; // not used
	for (;;)
	{
		ret = m_w2doc_map.TravelKey(word_id, temp_ptr, element_size); 
		if (ret)
		{
			break;
		}
		did_sets = (whl_list<doc_infer*>*)temp_ptr;
		ret = did_sets->del(dinfer);
		if (ret)
		{
			
		}
	}

	//3. del it from m_doc_infer 
	// 将static_array的最后一个元素和待删除元素交换，然后m_size-1
	doc_infer* tail = NULL;
	ret = m_doc_indexer.SearchKey_FL(((doc_infer*)(m_doc_infer.m_pointer + m_doc_infer.m_size - 1))->m_id, (var_vd**)&tail);
	if (ret)
	{

	}	
	dinfer->copy(tail);
	tail = dinfer;
	m_doc_infer.m_size--;
	
	//4. del it for doc inverted index
	ret = m_doc_indexer.DeleteKey_FL(_did);
	if (ret)
	{
		LOG_FAILE_CALL_RET("event_container::del_doc", "m_doc_indexer.DeleteKey_FL", ret);
		return RET_FALSE;
	}

	return RET_SECCEED;
}

var_4 event_container::del_evt(const var_u8 _evt_ID)
{
    m_lck.lock_w();
    try
    {
		event_infer* einfer = NULL;
		var_4 ret = m_evt_indexer.SearchKey_FL(_evt_ID, (var_vd**)&einfer);
		if (ret)
		{
			LOG_FAILE_CALL_RET("event_container::del_evt","m_evt_indexer.SearchKey_FL", ret);
			throw RET_FALSE;
		}
		
		//1. del doc
		whl_node<simple_pair<var_u8, var_u4> >* itr = einfer->m_did_rn_map.m_head;
		var_u4 idx = 0;
		for (; idx < einfer->m_did_rn_map.m_cnt; ++idx, itr = itr->_right)
		{
			ret = del_doc(itr->_val.left);
			if (ret)
			{
			
			}
		}

		//2. del self information
		m_w2evt_map.PreTravelKey();
		whl_list<event_infer*> *evt_sets = NULL;
		var_vd *temp_ptr = NULL;
		var_u8 word_id = 0;
		var_4 element_size = 0; // not used
		for (;;)
		{
			ret = m_w2evt_map.TravelKey(word_id, temp_ptr, element_size);
			if (ret)
			{
				break;
			}
			evt_sets = (whl_list<event_infer*>*)temp_ptr;
			ret = evt_sets->del(einfer);
			if (ret)
			{
			
			}
		}

		//3. del it from  m_evt_infer
		// 将static_array的最后一个元素和待删除元素交换，然后m_size-1
		event_infer* tail = NULL;
		ret = m_evt_indexer.SearchKey_FL(((event_infer*)(m_evt_infer.m_pointer + m_evt_infer.m_size - 1))->m_id, (var_vd**)&tail);
		if (ret)
		{
		
		}
		einfer->copy(tail);
		tail = einfer;
		m_evt_infer.m_size--;

		//4. del it from event inverted index
		ret = m_evt_indexer.DeleteKey_FL(_evt_ID);
		if (ret)
		{
			LOG_FAILE_CALL_RET("event_container::del_evt", "m_evt_indexer.DeleteKey_FL", ret);
			throw RET_FALSE;
		}
        throw RET_SECCEED;
    }
    catch (const var_4 _err_code)
    {
        m_lck.unlock();
        return _err_code;	
    }
}

var_vd event_container::clear()
{
    if (NULL != m_evt_allocator)
    {
        delete m_evt_allocator;
        m_evt_allocator = NULL;
    }

    m_doc_infer.m_size = 0;
    m_evt_infer.m_size = 0;
}

var_4 event_container::init(UC_ReadConfigFile* _reader, UT_Persistent_KeyValue<var_u8>* _storer, const var_4 _start_opt)
{
    //1. 启动
    //init m_dnode_allocator 
	assert(NULL == m_dnode_allocator);
    m_dnode_allocator = new UC_Allocator_Recycle;
    if (NULL == m_dnode_allocator)
    {
        LOG_FAILE_NEW("m_dnode_allocator");
        return EMGR_ERROR_INIT - 11;
    }

    m_dnode_size = sizeof(doc_infer);
    var_4 count = 10000;
    var_4 ret = m_dnode_allocator->initMem(m_dnode_size, count);
    if (0 != ret)
    {
        LOG_FAILE_CALL_RET("event_container::init", "m_dnode_allocator->initMem", ret);
        return EMGR_ERROR_INIT - 12;
    }
	//init m_enode_allocator
	assert(NULL == m_enode_allocator);
	m_enode_allocator = new UC_Allocator_Recycle;
	if (NULL == m_enode_allocator)
	{
		LOG_FAILE_NEW("m_enode_allocator");
		return EMGR_ERROR_INIT - 11;
	}

    m_enode_size = sizeof(event_infer);
    count = 10000;
    ret = m_enode_allocator->initMem(m_enode_size, count);
    if (0 != ret)
    {
        LOG_FAILE_CALL_RET("event_container::init", "m_enode_allocator->initMem", ret);
        return EMGR_ERROR_INIT - 12;
    }
	
	//init m_point_allocator
	assert(NULL == m_pointer_allocator);
	m_pointer_allocator = new UC_Allocator_Recycle;
	if (NULL == m_pointer_allocator)
	{
		LOG_FAILE_NEW("m_point_allocator");
		return EMGR_ERROR_INIT - 12;
	}
    m_pointer_size = sizeof(var_vd*);
    count = 10000;
    ret = m_pointer_allocator->initMem(m_pointer_size, count);
    if (0 != ret)
    {
        LOG_FAILE_CALL_RET("event_container::init", "m_pointer_allocator->initMem", ret);
        return EMGR_ERROR_INIT - 12;
    }
	// 
    ret = _reader->GetFieldValue("main_data_path", m_data_path);
    if (ret)
    {
        LOG_FAILE_CALL_RET("event_container::init", "get main_data_path", ret);
        return RET_ERROR_INVALID_PARAM;
    }

    ret = cp_create_dir(m_data_path);
    if (ret)
    {
        LOG_FAILE_CALL_RET("event_container::init", "::cp_create_dir", ret);
        return RET_ERROR_INVALID_PARAM;
    }

    var_u4 max_key_num = 4<<20;
	
	ret = m_wid2str_map.InitHashSearch(max_key_num, -1);// 变长hash
	if (ret)
	{
		LOG_FAILE_CALL_RET("event_container::init", "m_wid2str_map.init", ret);
		return RET_ERROR_INVALID_PARAM;
	}

    ret = m_w2doc_map.InitHashSearch(max_key_num, sizeof(whl_list<doc_infer*>));
    if (ret)
    {
        LOG_FAILE_CALL_RET("event_container::init", "m_w2did_map.init", ret);
        return RET_ERROR_INVALID_PARAM;
    }
    
    ret = m_w2evt_map.InitHashSearch(max_key_num, sizeof(whl_list<event_infer*>));
    if (ret)
    {
        LOG_FAILE_CALL_RET("event_container::init", "m_w2eid_map.init", ret);
        return RET_ERROR_INVALID_PARAM;
    }

    ret = m_cdd_docers.init(NULL, NULL, max_key_num);
    if (ret)
    {
        LOG_FAILE_CALL_RET("event_container::init", "m_cdd_docers.init", ret);
        return RET_ERROR_INVALID_PARAM;
    }

    ret = m_cdd_evters.init(NULL, NULL, max_key_num);
    if (ret)
    {
        LOG_FAILE_CALL_RET("event_container::init", "m_cdd_evters.init", ret);
        return RET_ERROR_INVALID_PARAM;
    }
	//init m_doc_infer
	var_4 max_doc_num = 0;
	ret = _reader->GetFieldValue("max_doc_num", max_doc_num);
	if (ret)
	{
	
	}
	m_doc_infer.m_size = 0;
	m_doc_infer.m_capacity = max_doc_num;
	m_doc_infer.m_pointer = (doc_infer*)malloc(sizeof(doc_infer)*max_doc_num);
	if (!m_doc_infer.m_pointer)
	{
		
	}
	doc_infer* dinfer = NULL;
	var_4 tag_cl_num = 0;
	var_4 tag_ar_num = 0;
	ret = _reader->GetFieldValue("tag_cl_num", tag_cl_num);
	if (ret)
	{
	
	}
	ret = _reader->GetFieldValue("tag_ar_num", tag_ar_num);
	if (ret)
	{
	
	}
	var_u4 idx = 0;
	for (; idx < m_doc_infer.m_capacity; ++idx)
	{
		dinfer = (doc_infer*)(m_doc_infer.m_pointer + idx);

		dinfer->m_tag_cl.m_size = 0;
		dinfer->m_tag_cl.m_capacity = tag_cl_num;
		dinfer->m_tag_cl.m_pointer = (var_u8*)malloc(sizeof(var_u8)*tag_cl_num);
		
		dinfer->m_tag_ar.m_size = 0;
		dinfer->m_tag_ar.m_capacity = tag_ar_num;
		dinfer->m_tag_ar.m_pointer = (var_u8*)malloc(sizeof(var_u8)*tag_ar_num);
	}
	// init m_doc_indexer
	ret = m_doc_indexer.InitHashSearch(max_doc_num, sizeof(doc_infer*));
	if (ret)
	{
	
	}
	// init m_evt_infer
	var_4 max_evt_num = 0;
	ret = _reader->GetFieldValue("max_evt_num", max_evt_num);
	if (ret)
	{
		
	}
	m_evt_infer.m_size = 0;
	m_evt_infer.m_capacity = max_evt_num;
	m_evt_infer.m_pointer = (event_infer*)malloc(sizeof(event_infer)*max_evt_num);
	if (!m_evt_infer.m_pointer)
	{
	
	}
	event_infer* einfer = NULL;
	for (idx = 0; idx < max_evt_num; ++idx)
	{
		einfer = (event_infer*)(m_evt_infer.m_pointer + idx);
	
		ret = einfer->m_did_rn_map.init(NULL, NULL, 1024);
		if (ret)
		{
		
		}
		ret = einfer->m_cl_rn_map.init(NULL, NULL, 10);
		if (ret)
		{
		
		}
		ret = einfer->m_ar_rn_map.init(NULL, NULL, 10);
		if (ret)
		{
		
		}
		ret = einfer->m_key_cnt_lst.init(NULL, NULL, 100);
		if (ret)
		{
		
		}
	}
	// init m_evt_indexer
	ret = m_evt_indexer.InitHashSearch(max_evt_num, sizeof(event_infer*));
	if (ret)
	{
	
	}
    //2. 恢复数据
    {
        var_1 name_sto1[256];
        var_1 name_sto2[256];
        var_1 name_sto3[256];
		sprintf(name_sto1, "%s/evt_inf.dat", m_data_path);
        sprintf(name_sto2, "%s/doc_inf.dat", m_data_path);
        sprintf(name_sto3, "%s/wid2str.map", m_data_path);
		if (!access(name_sto1, 0) && !access(name_sto2, 0) && !access(name_sto3, 0))
        {
            if (_start_opt == CLEAR_START)
            {
                remove(name_sto1);
                remove(name_sto2);
				remove(name_sto3);
            }
            else
            {
                ret = load(name_sto1, name_sto2, name_sto3);
                if (ret)
                {
                    return RET_ERROR_INVALID_PARAM;
                }
            }
        }
        else if (!access(name_sto1, 0))
        {
            remove(name_sto1);
        }
        else if (!access(name_sto2, 0))
        {
            remove(name_sto2);
        }
		else if (!access(name_sto3, 0))
		{
			remove(name_sto3);
		}
    }

    m_doc_storager = _storer;

    return RET_SECCEED;
}

var_4 event_container::save()
{
   	FILE* fh = NULL;
	var_1 new_path[256] = "";
	var_1 file_tmp[256] = "";
	try
	{
		m_lck.lock_r();
   		//1. save m_evt_infer    
		sprintf(new_path, "%s/evt_inf.dat", m_data_path);

		sprintf(file_tmp, "%s.tmp", new_path);
		fh = fopen(file_tmp, "wb");
		if (!fh)
		{
			LOG_FAILE_NEW(file_tmp);
			throw RET_ERROR_INVALID_PARAM;
		}

		var_4 ret = fwrite(&m_evt_infer.m_size, sizeof(m_evt_infer.m_size), 1, fh);
		if (ret != 1)
		{
			LOG_FAILE_CALL_RET("event_container::save", "fwrite m_evt_infer.m_size", ret);
			throw RET_ERROR_INVALID_PARAM;
		}

    	var_u4 idx = 0u;
		for (; m_evt_infer.m_size > idx; ++idx)
		{
			m_evt_infer.m_pointer[idx].serialize(fh);
		}
		fclose(fh);

		rename(file_tmp, new_path);
    
		//2. save m_doc_infer
		sprintf(new_path, "%s/doc_inf.dat", m_data_path);

		sprintf(file_tmp, "%s.tmp", new_path);
		fh = fopen(file_tmp, "wb");
		if (!fh)
		{
			LOG_FAILE_NEW(file_tmp);
			throw RET_ERROR_INVALID_PARAM;
		}

		if (fwrite(&m_doc_infer.m_size, sizeof(m_doc_infer.m_size), 1, fh) != 1)
		{
			LOG_FAILE_CALL_RET("event_container::save", "fwrite m_doc_infer.m_size", ret);
			throw RET_ERROR_INVALID_PARAM;
		}

		for (idx = 0u; m_doc_infer.m_size > idx; ++idx)
		{
			m_doc_infer.m_pointer[idx].serialize(fh);
		}
		fclose(fh);

		rename(file_tmp, new_path);

		//3. save m_wid2str_map
		sprintf(new_path, "%s/wid2str.map", m_data_path);
		
		sprintf(file_tmp, "%s.tmp", new_path);
		fh = fopen(file_tmp, "wb");
		if (!fh)
		{
			LOG_FAILE_NEW(file_tmp);
			throw RET_ERROR_INVALID_PARAM;
		}
		if (fwrite(&m_wid2str_map.m_lAllDataNum, sizeof(m_wid2str_map.m_lAllDataNum), 1, fh) != 1)
		{
			LOG_FAILE_CALL_RET("event_container::save", "fwrite m_wid2str_map.m_lAddDataNum", ret);
			throw RET_ERROR_INVALID_PARAM;
		}
		m_wid2str_map.PreTravelKey();
		
		var_vd* word_str = NULL;
		var_4 word_len = 0;
		var_u8 word_id = 0;
		for(;;)
		{
			ret = m_wid2str_map.TravelKey(word_id, word_str, word_len);
			if (ret)
			{
				break;
			}
			if (fwrite(&word_id, sizeof(word_id), 1, fh) != 1)
			{
				throw RET_ERROR_INVALID_PARAM;
			}
			if (fwrite(&word_len, sizeof(word_len), 1, fh) != 1)
			{
				throw RET_ERROR_INVALID_PARAM;
			}
			if (fwrite((var_1*)word_str, 1, word_len, fh) != word_len)
			{
				throw RET_ERROR_INVALID_PARAM;
			}
		}
		fclose(fh);
		rename(file_tmp, new_path);
		//	
		m_lck.unlock();
	}
	catch (const var_4 _err_code)
	{
		if (fh)
		{
			fclose(fh);
			fh = NULL;
		}
		m_lck.unlock();
		return _err_code;
	}
    return RET_SECCEED;
}

var_4 event_container::load(var_1* _evt_path, var_1* _doc_path, var_1* _wid2str_path)
{
    //1. load event infer
    FILE* fh = fopen(_evt_path, "rb");
    if (!fh)
    {
        return RET_ERROR_INVALID_PARAM;
    }

    var_4 ret = fread(&m_evt_infer.m_size, sizeof(m_evt_infer.m_size), 1, fh);
    if (1 != ret)
    {
        fclose(fh);
        return RET_ERROR_INVALID_PARAM;
    }

    if (m_evt_infer.m_size > m_evt_infer.m_capacity)
    {
		// !!!重新分配
    }

    var_u4 idx = 0u;
    for (; m_evt_infer.m_size > idx; ++idx)
    {
        ret = m_evt_infer.m_pointer[idx].deserialize(fh);
        if (ret)
        {
            fclose(fh);
            return RET_ERROR_INVALID_PARAM;
        }
    }
    fclose(fh);

    //2. load doc infer
    fh = fopen(_doc_path, "rb");
    if (!fh)
    {
        return RET_ERROR_INVALID_PARAM;
    }

    ret = fread(&m_doc_infer.m_size, sizeof(m_doc_infer.m_size), 1, fh);
    if (1 != ret)
    {
        fclose(fh);
        return RET_ERROR_INVALID_PARAM;
    }

    if (m_doc_infer.m_size > m_doc_infer.m_capacity)
    {
		// !!!重新分配
    }

    for (idx = 0u; m_doc_infer.m_size > idx; ++idx)
    {
        ret = m_doc_infer.m_pointer[idx].deserialize(fh);
        if (ret)
        {
            fclose(fh);
            return RET_ERROR_INVALID_PARAM;
        }
    }
    fclose(fh);

	//3. load m_wid2str_map
	fh = fopen(_wid2str_path, "rb");
	if (!fh)
	{
		return RET_ERROR_INVALID_PARAM;
	}

	ret = fread(&m_wid2str_map.m_lAllDataNum, sizeof(m_wid2str_map.m_lAllDataNum), 1, fh);
	if (ret != 1)
	{
		fclose(fh);
		return RET_ERROR_INVALID_PARAM;
	}

	if (m_wid2str_map.m_lAllDataNum > m_wid2str_map.m_lHashTableSize)
	{
		// !!!重新分配	
	}
	var_u4 word_len = 0;
	var_u8 word_id = 0;
	var_1  word_str[128] = "";
	for (idx = 0; idx < m_wid2str_map.m_lAllDataNum; ++idx)
	{
		ret = fread(&word_id, sizeof(word_id), 1, fh);
		if (ret != 1)
		{
			fclose(fh);
			return RET_ERROR_INVALID_PARAM;
		}
		ret = fread(&word_len, sizeof(word_len), 1, fh);
		if (ret != 1)
		{
			fclose(fh);
			return RET_ERROR_INVALID_PARAM;
		}
		ret = fread(word_str, 1, word_len, fh);
		if (ret != word_len)
		{
			fclose(fh);
			return RET_ERROR_INVALID_PARAM;
		}
		ret = m_wid2str_map.AddKey_VL(word_id, word_str, word_len, NULL, NULL);
		if (ret < 0)
		{
			fclose(fh);
			LOG_FAILE_CALL_RET("event_container::load", "m_wid2str_map.AddKey_VL", ret);
			return RET_ERROR_INVALID_PARAM;
		}
	}
	fclose(fh);

	//4. traversal m_evt_infer and recreate m_evt_indexer 
   	event_infer* einfer = NULL;
	for (idx = 0; idx < m_evt_infer.m_size; ++idx)
	{
		einfer = (event_infer*)(m_evt_infer.m_pointer + idx);
		ret = m_evt_indexer.AddKey_FL(einfer->m_id, (var_vd*)&einfer, NULL);
		if (ret < 0)
		{
			LOG_FAILE_CALL_RET("event_container::load", "m_evt_indexer.AddKey_FL", ret);
			return RET_ERROR_INVALID_PARAM;
		}
	}

	//5. traversal m_doc_infer and recreate m_doc_indexer m_w2doc_map m_w2evt_map
	doc_infer* dinfer = NULL;
	for (idx = 0; idx < m_doc_infer.m_size; ++idx)
	{
		dinfer = (doc_infer*)(m_doc_infer.m_pointer + idx);
		ret = m_doc_indexer.AddKey_FL(dinfer->m_id, (var_vd*)&dinfer, NULL);
		if (ret < 0)
		{
			LOG_FAILE_CALL_RET("event_container::load", "m_doc_indexer.AddKey_FL", ret);
			return RET_ERROR_INVALID_PARAM;
		}
		// recreate m_w2doc_map and m_w2evt_map
		update_key_list(dinfer);
	}
	//6. m_wid2str_map need be saved

    return RET_SECCEED;
}

var_4 event_container::boult(var_u8 word_id, var_u8 tm)
{   
    // 注意上锁时间最小化
    // 按时间淘汰m_w2doc_map倒排表
    // m_evt_infer 淘汰过期小堆

    m_lck.lock_w();
	whl_list<doc_infer* >* did_sets = NULL;
	var_4 ret = m_w2doc_map.SearchKey_FL(word_id, (var_vd**)&did_sets);
	if (ret)
	{
		return RET_FALSE;
	}
	whl_node<doc_infer* >* itr = did_sets->m_head;
	whl_node<doc_infer* >* pre = NULL;
	for (;;)
	{
		if (!itr)
		{
			break;
		}
		if (itr->_val->m_tm < tm - 86400 * 3)
		{
			pre = itr->_right;
			ret = did_sets->del(itr->_val);
			if (ret)
			{
				LOG_FAILE_CALL_RET("event_container::boult", "did_sets->del", ret);
				return RET_FALSE;
			}
			itr = pre;
		}
		else
		{
			itr = itr->_right;
		}
	}

	whl_list<event_infer* >* evt_sets = NULL;
	ret = m_w2evt_map.SearchKey_FL(word_id, (var_vd**)&evt_sets);
	if (ret)
	{
		return RET_FALSE;
	}
	whl_node<event_infer* >* another_itr = evt_sets->m_head;
	whl_node<event_infer* >* another_pre = NULL;
	var_u4 idx = 0;
	for (; idx < evt_sets->m_cnt; ++idx, another_itr = another_itr->_right)
	{
		if ((var_vd*)another_itr->_val == m_evt_ptr)
		{
			break;
		}
		if (another_itr->_val->m_doc_cnt >= 50)
		{
			another_pre = another_itr;
		}
	}
	if (another_pre)
	{
		for (another_itr = another_pre->_right; another_itr; another_itr = another_itr->_right)
		{
			if (another_itr->_val->m_time >= another_pre->_val->m_time - 86400 * 3)
			{
				continue;
			}
			if (another_itr->_val->m_time < ((event_infer*)m_evt_ptr)->m_time - 86400 * 3)
			{
				break;
			}
			if (another_itr->_val->m_doc_cnt < MIN_DOC_NUM)
			{
				ret = evt_sets->del(another_itr->_val);
			}
		}
		m_evt_ptr = (var_vd*)another_pre->_val;
	}
	m_lck.unlock();
    return RET_SECCEED;
}

var_f4 event_container::measure_docdist(doc_infer* _left, doc_infer* _right)
{
    //!!! calculate the distance of two docs
	var_4 i, j, tm_dst;
	var_d8 tw, doc_dst = 0;

	if ((tm_dst = abs(_left->m_tm - _right->m_tm)) > 3*86400)
	{
		return (var_f4)RET_SECCEED;
	}
	tw = pow((1 + tm_dst), -0.25);
	
	for(i = 0; i < _left->m_terms.m_size; ++i)
	{
		term_info_st &lterm = _left->m_terms.m_pointer[i];
		for (j = 0; j < _right->m_terms.m_size; ++j)
		{
			term_info_st &rterm = _right->m_terms.m_pointer[j];
			if (lterm.word_id == rterm.word_id)
			{
				doc_dst += lterm.weight * rterm.weight * tw;
			}
		}
	}
    return (var_f4)doc_dst;
}


