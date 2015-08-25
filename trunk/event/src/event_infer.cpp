#include "event_infer.h"

event_infer::event_infer()
{
	
}
	
event_infer::~event_infer()
{

}


var_4 event_infer::serialize(FILE* _fh)
{
	// serialize 和 deserialize保持一致
	//
	var_4 ret = fwrite(&m_doc_cnt, sizeof(m_doc_cnt), 1, _fh);
	if (ret != 1)
	{
		return RET_FALSE;
	}
	
	ret = fwrite(&m_id, sizeof(m_id), 1, _fh);
	if (ret != 1)
	{
		return RET_FALSE;
	}

	ret = fwrite(&m_key_did, sizeof(m_key_did), 1, _fh);
	if (ret != 1)
	{
		return RET_FALSE;
	}

	ret = fwrite(&m_time, sizeof(m_time), 1, _fh);
	if (ret != 1)
	{
		return RET_FALSE;
	}
	//
	var_4 idx = 0;
	whl_node<simple_pair<var_u8, var_u4> >* itr = m_did_rn_map.m_head;
	ret = fwrite(&m_did_rn_map.m_cnt, sizeof(m_did_rn_map.m_cnt), 1, _fh);
	if (ret != 1)
	{
		return RET_FALSE;
	}
	for (; idx < m_did_rn_map.m_cnt; ++idx, itr = itr->_right)
	{
		if (fwrite(&itr->_val, sizeof(itr->_val), 1, _fh) != 1)
		{
			return RET_FALSE;
		}
	}
	//
	idx = 0, itr = m_ar_rn_map.m_head;
	if (fwrite(&m_ar_rn_map.m_cnt, sizeof(m_ar_rn_map.m_cnt), 1, _fh) != 1)
	{
		return RET_FALSE;
	}
	for (; idx < m_ar_rn_map.m_cnt; ++idx, itr = itr->_right)
	{
		if (fwrite(&itr->_val, sizeof(itr->_val), 1, _fh) != 1)
		{
			return RET_FALSE;
		}
	}
	//
	idx = 0, itr = m_cl_rn_map.m_head;
	if (fwrite(&m_cl_rn_map.m_cnt, sizeof(m_cl_rn_map.m_cnt), 1, _fh) != 1)
	{
		return RET_FALSE;
	}
	for (; idx < m_cl_rn_map.m_cnt; ++idx, itr = itr->_right)
	{
		if (fwrite(&itr->_val, sizeof(itr->_val), 1, _fh) != 1)
		{
			return RET_FALSE;
		}
	}
	//
	whl_node<simple_pair<var_u8, simple_pair<var_u4, var_f4> > >* another_itr;
	idx = 0, another_itr = m_key_cnt_lst.m_head;
	if (fwrite(&m_key_cnt_lst.m_cnt, sizeof(m_key_cnt_lst.m_cnt), 1, _fh) != 1)
	{
		return RET_FALSE;
	}
	for (; idx < m_key_cnt_lst.m_cnt; ++idx, another_itr = another_itr->_right)
	{
		if (fwrite(&another_itr->_val, sizeof(another_itr->_val), 1, _fh) != 1)
		{
			return RET_FALSE;
		}
	}
	//
	return RET_SECCEED;
}

var_4 event_infer::deserialize(FILE* _fh)
{
	// serialize 和 deserialize保持一致
	//
	var_4 ret = fread(&m_doc_cnt, sizeof(m_doc_cnt), 1, _fh);
	if (ret != 1)
	{
		return RET_FALSE;
	}

	ret = fread(&m_id, sizeof(m_id), 1, _fh);
	if (ret != 1)
	{
		return RET_FALSE;
	}

	ret = fread(&m_key_did, sizeof(m_key_did), 1, _fh);
	if (ret != 1)
	{
		return RET_FALSE;
	}

	ret = fread(&m_time, sizeof(m_time), 1, _fh);
	if (ret != 1)
	{
		return RET_FALSE;
	}

	//
	var_4 idx = 0;
	whl_node<simple_pair<var_u8, var_u4> >* itr = m_did_rn_map.m_head;
	ret = fread(&m_did_rn_map.m_cnt, sizeof(m_did_rn_map.m_cnt), 1, _fh);
	if (ret != 1)
	{
		return RET_FALSE;
	}
	for (; idx < m_did_rn_map.m_cnt; ++idx, itr = itr->_right)
	{
		ret = fread(&itr->_val, sizeof(itr->_val), 1, _fh);
		if (ret != 1)
		{
			return RET_FALSE;
		}
	}
	//
	idx = 0, itr = m_ar_rn_map.m_head;
	ret = fread(&m_ar_rn_map.m_cnt, sizeof(m_ar_rn_map.m_cnt), 1, _fh);
	if (ret != 1)
	{
		return RET_FALSE;
	}
	for (; idx < m_ar_rn_map.m_cnt; ++idx, itr = itr->_right)
	{
		ret = fread(&itr->_val, sizeof(itr->_val), 1, _fh);
		if (ret != 1)
		{
			return RET_FALSE;
		}
	}
	//
	idx = 0, itr = m_cl_rn_map.m_head;
	ret = fread(&m_cl_rn_map.m_cnt, sizeof(m_cl_rn_map.m_cnt), 1, _fh);
	if (ret != 1)
	{
		return RET_FALSE;	
	}
	for (; idx < m_cl_rn_map.m_cnt; ++idx, itr = itr->_right)
	{
		ret = fread(&itr->_val, sizeof(itr->_val), 1, _fh);
		if (ret != 1)
		{
			return RET_FALSE;	
		}
	}
	//
	whl_node<simple_pair<var_u8, simple_pair<var_u4, var_f4> > >* another_itr;
	idx = 0, another_itr = m_key_cnt_lst.m_head;
	if (fread(&m_key_cnt_lst.m_cnt, sizeof(m_key_cnt_lst.m_cnt), 1, _fh) != 1)
	{
		return RET_FALSE;
	}
	for (; idx < m_key_cnt_lst.m_cnt; ++idx, another_itr = another_itr->_right)
	{
		ret = fread(&another_itr->_val, sizeof(another_itr->_val), 1, _fh);
		if (ret != 1)
		{
			return RET_FALSE;
		}
	}
	//
	return RET_SECCEED;
}
	
bool m_did_rn_map_equal(simple_pair<var_u8, var_u4>& lhs, simple_pair<var_u8, var_u4>& rhs)
{
	return (lhs.left == rhs.left);
}

var_4 event_infer::update(doc_infer* _dinfer,var_u4 _new)
{
	//1.  m_key_cnt_lst
	//2.  m_did_rn_map
	//3.  adjust m_key_did
	//4.  update key word inverted index	

	var_u4 idx = 0u, ret = 0;
	
	{//1. update m_did_rn_map
		simple_pair<var_u8, var_u4> tmp(_dinfer->m_id, 1);
		whl_node< simple_pair<var_u8, var_u4> >* pre = m_did_rn_map.query(tmp);
		if (pre)
		{
			++pre->_val.right;
		}
		else
		{
			ret = m_did_rn_map.add(tmp);
			if (ret)
			{
		    	LOG_FAILE_CALL_RET("event_infer::update", "m_did_rn_map.add()", ret);
				return RET_FALSE;
			}
		}
	}
	{//2. update m_ar_rn_map
		whl_node< simple_pair<var_u8, var_u4> >* pre = NULL;
		simple_pair<var_u8, var_u4> tmp;
		for (; idx < _dinfer->m_tag_ar.m_size; ++idx)	
		{
			var_u8 word_id = _dinfer->m_tag_ar.m_pointer[idx];
			tmp.left = word_id;
			tmp.right = 1;
			pre = m_ar_rn_map.query(tmp);
			if (pre)
			{
				++pre->_val.right;
			}
			else
			{
				ret = m_ar_rn_map.add(tmp);
				if (ret)
				{
					LOG_FAILE_CALL_RET("event_infer::update", "m_ar_rn_map.add()", ret);
					return RET_FALSE;
				}
			}
		}
	}

	{//3. update m_cl_rn_map 
		whl_node< simple_pair<var_u8, var_u4> >* pre = NULL;
		simple_pair<var_u8, var_u4> tmp;
		for (; idx < _dinfer->m_tag_cl.m_size; ++idx)
		{
			var_u8 word_id = _dinfer->m_tag_cl.m_pointer[idx];
			tmp.left  = word_id;
			tmp.right = 1;
			pre = m_cl_rn_map.query(tmp);
			if (pre)
			{
				++pre->_val.right;
			}
			else
			{
				ret = m_cl_rn_map.add(tmp);
				if (ret)
				{
					LOG_FAILE_CALL_RET("event_infer::update", "m_cl_rn_map.add()", ret);
					return RET_FALSE;
				}
			}
		}
	}

	{//4. update m_key_cnt_lst 
		simple_pair<var_u8, simple_pair<var_u4, var_f4> > tmp;
		whl_node< simple_pair<var_u8, simple_pair<var_u4, var_f4> > >*pre = NULL;
		term_info_st* cutr_terms = _dinfer->m_terms.m_pointer;
		for (; _dinfer->m_terms.m_size > idx; ++idx)
		{
			tmp.left = cutr_terms[idx].word_id;
			tmp.right.left = 1;
			tmp.right.right = cutr_terms[idx].idf;
			pre = m_key_cnt_lst.query(tmp);
			if (pre)
			{
				++pre->_val.right.left;
			}
			else
			{
				ret = m_key_cnt_lst.add(tmp);
				if (ret)
				{
					LOG_FAILE_CALL_RET("event_infer::update", "m_key_cnt_lst.add()", ret);
					return RET_FALSE;
				}
			}
		}
	}
	if (_new)
	{
		++m_doc_cnt;
	}

	return  RET_SECCEED;
}

var_4 event_infer::regist(doc_infer* _dinfer)
{
	update(_dinfer, 1);
	return RET_SECCEED;
}

var_4 event_infer::copy(event_infer* &_einfer)
{
	m_doc_cnt = _einfer->m_doc_cnt;
	m_id = _einfer->m_id;
	m_key_did = _einfer->m_key_did;
	m_time = _einfer->m_time;

	m_did_rn_map.reset();
	var_u4 idx = 0;
	whl_node<simple_pair<var_u8, var_u4> >* itr = _einfer->m_did_rn_map.m_head;

	for (; idx < _einfer->m_did_rn_map.m_cnt; ++idx, itr = itr->_right)
	{
		m_did_rn_map.add(itr->_val);
	}

	m_ar_rn_map.reset();
	idx = 0, itr = _einfer->m_ar_rn_map.m_head;
	for (; idx < _einfer->m_ar_rn_map.m_cnt; ++idx, itr = itr->_right)
	{
		m_ar_rn_map.add(itr->_val);
	}

	m_cl_rn_map.reset();
	idx = 0, itr = _einfer->m_cl_rn_map.m_head;
	for (; idx < _einfer->m_cl_rn_map.m_cnt; ++idx, itr = itr->_right)
	{
		m_cl_rn_map.add(itr->_val);
	}

	m_key_cnt_lst.reset();
	whl_node<simple_pair<var_u8, simple_pair<var_u4, var_f4> > >* another_itr = _einfer->m_key_cnt_lst.m_head;
	for (idx = 0; idx < _einfer->m_key_cnt_lst.m_cnt; ++idx, another_itr = another_itr->_right)
	{
		m_key_cnt_lst.add(another_itr->_val);
	}
	return RET_SECCEED;
}

