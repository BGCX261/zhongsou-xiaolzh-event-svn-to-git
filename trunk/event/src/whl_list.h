#ifndef __WHL_LIST_H_
#define __WHL_LIST_H_

#include "UH_Define.h"
#include "UC_Allocator_Recycle.h"

template<typename T>
struct whl_node
{
    T _val;
    whl_node* _left;
    whl_node* _right;
};

// µ¥Ïß³Ì
template<typename T>
class whl_list
{
public:
    whl_list()
    //: m_cmp(NULL)
    : m_head(NULL)
    , m_cnt(0u)
    , m_allocor(NULL)
    , m_self_allocor(0)
    {
        ;
    }
	
    ~whl_list()
    {
        if (m_self_allocor && m_allocor)
        {
            delete m_allocor;
            m_allocor = NULL;
        }
    }

    var_4 init(var_4 (*_cmp)(T& _lhs, T& rhs), var_4 (*_eql)(T& _lhs, T& rhs), var_u4 _max_size, UC_Allocator_Recycle* _allocor = NULL)
    {
        m_allocor = _allocor;
        m_self_allocor = 0;
        
        if (!m_allocor)
        {
            m_self_allocor = 1;
            m_allocor = new UC_Allocator_Recycle;
            var_4 ret = m_allocor->initMem(_max_size, sizeof(whl_node<T>));
            if (ret)
            {
                delete m_allocor;
                m_allocor = NULL;
                return -1;
            }
        }

        m_cmp = _cmp;
        m_eql = _eql;

        m_cnt = 0;

        return 0;
    }
	// insertion sort
    var_4 add(T& _val)
    {
        whl_node<T>* ret = query(_val);
        if (ret)
            return 1;

        whl_node<T>* cur_node = (whl_node<T>*)m_allocor->AllocMem(); 
        cur_node->_val = _val;
        if (!m_head)
        {
            m_head = cur_node;
            ++m_cnt;
            return 0;
        }

        whl_node<T>* next_node = m_head;
        for (; ; next_node = next_node->_right)
        {
            if (!m_cmp(next_node->_val, _val))
            {
                break;   
            }             
            if (next_node->_right)
            {
                break;
            }
        }
        
        if (next_node == m_head)
        {
            cur_node->_left = next_node->_left;
            cur_node->_right = next_node;

            next_node->_left = cur_node;
        }
        else if (next_node->_right)
        {// middle
            cur_node->_left = next_node->_left;
            cur_node->_right = next_node;
            
            next_node->_left = cur_node;
            next_node->_left->_right = cur_node;
        }
        else
        {// tail
            next_node->_right = cur_node;

            cur_node->_right = NULL;
            cur_node->_left = next_node;
        }        

        ++m_cnt;

        return 0;
    }

    var_4 del(T& _val)
    {
        whl_node<T>* ret = query(_val);
        if (!ret)
            return -1;

        if (!ret->_left)
        {// head
            m_head = m_head->_right;
            m_head->_left = NULL;   
        }
        else if (!ret->_right)
        {// tail
            assert(ret->_left);
            ret->_left->_right = NULL;
        }
        else
        {// middle
            assert(ret->_left);
            assert(ret->_right);
            ret->_left->_right = ret->_right;
            ret->_right->_left = ret->_left;
        }  

        m_allocor->FreeMem((var_1*)ret);
        --m_cnt;

        return 0;
    }

    whl_node<T>* query(T& _val)
    {
        whl_node<T>* ret = m_head;
        
        for (; ret; ret = ret->_right)
        {
            if (m_eql(_val, ret->_val))
                break;
        }

        return ret;
    }

    var_4 adjust(T& _val)
    {
        var_4 ret = del(_val);
        if (ret)
            return -1;
        ret = add(_val);
		if (ret)
            return -2;
        return 0;
    }

    var_vd reset()
    {
        //m_cnt = 0u;
        for (; m_head; m_head = m_head->_right, --m_cnt)
        {
            m_allocor->FreeMem((var_1*)m_head);
        }
        assert(!m_cnt);
    }

public:
    var_4 (*m_cmp)(T& _lhs, T& rhs);
    var_4 (*m_eql)(T& _lhs, T& rhs);

    whl_node<T>* m_head;

    var_u4       m_cnt;   
    
    UC_Allocator_Recycle* m_allocor;

    var_4 m_self_allocor;
};

#endif
