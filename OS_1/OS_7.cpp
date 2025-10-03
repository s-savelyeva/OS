#include <iostream>
#include <Windows.h>
#include <locale>
using namespace std;

struct tree {
	int data;
	tree* left;
	tree* right;
};

struct ThreadData {
	tree* root;
	int sum;
};

void add(int data, tree*& tr) {
	if (!tr) {
		tr = new tree;
		(*tr).left = NULL;
		(*tr).right = NULL;
		(*tr).data = data;
	}
	else {
		if ((*tr).data > data) {
			add(data, (*tr).left);
		}
		else {
			add(data, (*tr).right);
		}
	}
}

DWORD WINAPI tree_sum(LPVOID param) {
	ThreadData* threadData = (ThreadData*)param;

	if (threadData->root == NULL) {
		threadData->sum = 0;
		return 0;
	}

	int sum = threadData->root->data;

	HANDLE hLeft = NULL, hRight = NULL;
	ThreadData leftData{ (threadData->root->left), 0 };
	ThreadData rightData{ (threadData->root->right), 0 };

	if (threadData->root->left) {
		hLeft = CreateThread(NULL, 0, tree_sum, &leftData, 0, NULL);
	}

	if (threadData->root->right) {
		hRight = CreateThread(NULL, 0, tree_sum, &rightData, 0, NULL);
	}

	HANDLE arr[2];
	int count = 0;
	if (hLeft) arr[count++] = hLeft;
	if (hRight) arr[count++] = hRight;

	if (count > 0) { WaitForMultipleObjects(count, arr, TRUE, INFINITE); }

	if (hLeft) CloseHandle(hLeft);
	if (hRight) CloseHandle(hRight);

	sum += leftData.sum + rightData.sum;

	threadData->sum = sum;
	return 0;
}

int main() {

	setlocale(LC_ALL, "Rus");

	int data = 1;
	tree* root = NULL;

	while (data != 0) {
		cout << "Введите элемент дерева (0 для выхода) = ";
		cin >> data;
		add(data, root);
	}

	ThreadData threadData{ root, 0 };
	HANDLE hThread = CreateThread(NULL, 0, tree_sum, &threadData, 0, NULL);

	WaitForSingleObject(hThread, INFINITE);
	CloseHandle(hThread);

	cout << "Сумма элементов дерева = " << threadData.sum << endl;

	return 0;
}
