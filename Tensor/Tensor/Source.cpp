#include <iostream>
#include <utility>

// �p�����[�^�p�b�N����ɂȂ�����I��
void print() {}

// �ЂƂȏ�̃p�����[�^���󂯎��悤�ɂ��A
// �ψ�����擪�Ƃ���ȊO�ɕ�������
template <class Head, class... Tail>
void print(Head head, Tail... tail)
{
	std::cout << head << std::endl;

	// �p�����[�^�p�b�Ntail�������head��tail�ɕ�������
	print(tail...);
	//print(std::move(tail)...);
}

int main()
{
	print(2, 'a', 3, 4, 5,3.4, "hello");
}