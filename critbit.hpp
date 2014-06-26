/*
	original code is https://github.com/agl/critbit
*/
#pragma once
#include <stdint.h>
#include <string.h>
#include <new>
#include <string>
#include <cybozu/bit_operation.hpp>

namespace critbit {

struct StrSet {
	StrSet()
		: root_(0)
		, size_(0)
	{
	}
	~StrSet()
	{
		if (!root_) return;
		clearAll(root_);
	}
	size_t size() const { return size_; }
	bool insert(const std::string& str)
	{
		return append(str.c_str(), str.size());
	}
	bool append(const char *s, size_t len = 0)
	{
		const uint8_t* u = reinterpret_cast<const uint8_t*>(s);
		if (len == 0) len = strlen(s);
		if (!root_) {
			root_ = (uint8_t*)malloc(len + 1);
			if (root_ == 0) throw std::bad_alloc();
			memcpy(root_, u, len + 1);
			size_++;
			return true;
		}
		uint8_t *p = seekToLeaf(root_, u, len);

		uint32_t newLen;
		uint8_t extra;

		for (newLen = 0; newLen < len; newLen++) {
			if (p[newLen] != u[newLen]) {
				extra = p[newLen] ^ u[newLen];
				goto different_byte_found;
			}
		}
		if (p[newLen]) {
			extra = p[newLen];
			goto different_byte_found;
		}
		return false; // already has s
	different_byte_found:
		extra = getBsr(extra);
		const uint8_t c = p[newLen];
		const int newdirection = (c >> extra) & 1;

		Node* const newNode = (Node*)malloc(sizeof(Node));
		if (newNode == 0) throw std::bad_alloc();

		uint8_t *const x = (uint8_t*)malloc(len + 1);
		if (x == 0) {
			free(newNode);
			throw std::bad_alloc();
		}
		memcpy(x, u, len + 1);
		newNode->len_ = newLen;
		newNode->extra_ = extra;
		newNode->child_[1 - newdirection] = x;
		uint8_t** wherep = &root_;

		for (;;) {
			uint8_t* p = *wherep;

			if (!isNode(p)) break;
			Node* q = (Node*)(p - 1);

			if (q->len_ > newLen) break;
			if (q->len_ == newLen && q->extra_ < extra) break;
			const int direction = q->getDirection(u, len);
			wherep = &q->child_[direction];
		}

		newNode->child_[newdirection] = *wherep;
		*wherep = (uint8_t*)(1 + (char*)newNode);
		size_++;
		return true;
	}
	bool has(const std::string& str) const
	{
		return has(str.c_str(), str.size());
	}
	bool has(const char *s, size_t len = 0) const
	{
		if (!root_) return false;
		const uint8_t* u = reinterpret_cast<const uint8_t*>(s);
		if (len == 0) len = strlen(s);
		uint8_t *p = seekToLeaf(root_, u, len);
		return strcmp(s, (const char*)p) == 0;
	}
	bool remove(const std::string& str)
	{
		return remove(str.c_str(), str.size());
	}
	bool remove(const char *s, size_t len = 0)
	{
		if (!root_) return false;
		const uint8_t* u = reinterpret_cast<const uint8_t*>(s);
		if (len == 0) len = strlen(s);
		uint8_t* p = root_;
		uint8_t** wherep = &root_;
		uint8_t** whereq = 0;
		Node* q = 0;
		int direction = 0;

		while (isNode(p)) {
			q = (Node*)(p - 1);
			direction = q->getDirection(u, len);
			whereq = wherep;
			wherep = &q->child_[direction];
			p = *wherep;
		}
		if (strcmp(s, (const char*)p) != 0) return false;
		size_--;
		free(p);
		if (!whereq) {
			root_ = 0;
			return true;
		}
		*whereq = q->child_[1 - direction];
		free(q);
		return true;
	}
	/*
		search string having prefix
		stop traversing if handler returns false
	*/
	bool traverse(const char* prefix, void *arg, bool handler(void *, const char*))
	{
		if (!root_) return 1;
		const uint8_t* u = reinterpret_cast<const uint8_t*>(prefix);
		const size_t len = strlen(prefix);
		uint8_t* p = root_;
		uint8_t* top = p;

		while (isNode(p)) {
			Node* q = (Node*)(p - 1);
			p = q->child_[q->getDirection(u, len)];
			if (q->len_ < len) top = p;
		}
		for (size_t i = 0; i < len; i++) {
			if (p[i] != u[i]) return 1;
		}
		return traverseInner(top, arg, handler);
	}
	void put() const
	{
		putInner(root_);
	}
private:
	uint8_t *root_;
	size_t size_;
	struct Node {
		uint8_t* child_[2];
		uint32_t len_;
		uint8_t extra_;
		int getDirection(const uint8_t *u, size_t len) const
		{
			if (len_ >= len) return 0;
			const uint8_t c = u[len_];
			return (c >> extra_) & 1;
		}
	};
	void putSp(size_t len) const
	{
		for (size_t i = 0; i < len; i++) {
			putchar(' ');
		}
	}
	void putInner(const uint8_t *p, size_t level = 0) const
	{
		putSp(level);
		if (!isNode(p)) {
			printf("[leaf] %s\n", p);
			return;
		}
		const Node *q = (const Node*)(p - 1);
		putSp(level); printf("len=%d, extra=%d\n", q->len_, q->extra_);
		putSp(level); printf("[L]\n"); putInner(q->child_[0], level + 2);
		putSp(level); printf("[R]\n"); putInner(q->child_[1], level + 2);
	}
	static inline bool isNode(const uint8_t *p)
	{
		return ((intptr_t)p & 1) != 0;
	}
	uint8_t *seekToLeaf(uint8_t *p, const uint8_t *u, size_t len) const
	{
		while (isNode(p)) {
			Node* q = (Node*)(p - 1);
			const int direction = q->getDirection(u, len);
			p = q->child_[direction];
		}
		return p;
	}

	static void clearAll(uint8_t* p)
	{
		if (isNode(p)) {
			Node* q = (Node*)(p - 1);
			clearAll(q->child_[0]);
			clearAll(q->child_[1]);
			free(q);
		} else {
			free(p);
		}
	}
	template<class T>
	inline bool traverseInner(uint8_t* p, T *arg, bool handler(T *, const char*))
	{
		if (isNode(p)) {
			Node* q = (Node*)(p - 1);

			if (!traverseInner(q->child_[0], arg, handler)) return false;
			return traverseInner(q->child_[1], arg, handler);
		}
		return handler(arg, (const char*)p);
	}
	/*
		0 for x = 1
		1 for x in [2, 3]
		2 for x in [4, 7]
		3 for x in [8, 15]
		4 for x in [16, 31]
		5 for x in [32, 63]
		6 for x in [64, 127]
		7 for x in [128, 255]
	*/
	uint8_t getBsr(uint8_t x) const
	{
		assert(x);
		return (uint8_t)cybozu::bsr(x);
	}
};

} // critbit
