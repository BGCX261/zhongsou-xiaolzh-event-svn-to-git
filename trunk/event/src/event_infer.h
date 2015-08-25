#ifndef __EVENT_INFER_H_
#define __EVENT_INFER_H_
#include "UH_Define.h"
#include "word_extractor_API.h"
#include "common_def.h"
#include "utility.h"
#include "whl_list.h"
#include "doc_infer.h"

class event_infer
{
public:
	event_infer();
	
	~event_infer();

	var_4 deserialize(
		FILE* _fh
		);
	
	var_4 serialize(
		FILE* _fh
		);

	var_4 regist(
		doc_infer* _dinfer
		);

	var_4 update( 
		doc_infer* _dinfer,
		var_u4 _new
		);
	var_4 copy(
		event_infer*&	
		);
public:

	var_u4 m_doc_cnt;
		
	var_u8 m_id;

	// primary doc id
	// 从m_did_rn_map前n个投票选出
	var_u8 m_key_did;

	// 求最新doc的时间
	var_u8 m_time;
	
	// doc id->repeat number
	// sort by repeat number
	// assuming most 1024 docs belong to this event
	whl_list< simple_pair<var_u8, var_u4> > m_did_rn_map;

	// area id->repeat number
	// top 10 sort by repeat number
	whl_list< simple_pair<var_u8, var_u4> > m_ar_rn_map;

	// class id->repeat number
	// top 10 sort by repeat number
	whl_list< simple_pair<var_u8, var_u4> > m_cl_rn_map;

	// key word-><count, idf>
	// top 100 sort by count 
	whl_list< simple_pair<var_u8, simple_pair<var_u4, var_f4> > > m_key_cnt_lst;
	
};

#endif

