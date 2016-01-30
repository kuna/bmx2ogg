#pragma once

#include <stdlib.h>

template <typename T>
class DynamicBuffer {
private:
	T *buffer;
	unsigned int buf_size;
public:
	DynamicBuffer();
	DynamicBuffer(unsigned int size);
	~DynamicBuffer();
	unsigned int GetSize();
	T* GetPtr();
};

#define MAX_BUFFER_SIZE	102400		// 100kb;

template <typename T>
DynamicBuffer<T>::DynamicBuffer() : DynamicBuffer(MAX_BUFFER_SIZE) {}

template <typename T>
DynamicBuffer<T>::DynamicBuffer(unsigned int size) : buf_size(size) {
	if (!buf_size)
		throw;
	buffer = (T*)malloc(sizeof(T) * size);
}

template <typename T>
DynamicBuffer<T>::~DynamicBuffer() {
	free(buffer);
}

template <typename T>
unsigned int DynamicBuffer<T>::GetSize() {
	return buf_size;
}

template <typename T>
T* DynamicBuffer<T>::GetPtr() {
	return buffer;
}
