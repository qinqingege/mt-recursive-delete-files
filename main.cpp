#include <iostream>
#include <thread>
#include <atomic>
#include <list>
#include <windows.h>

using namespace std;

atomic_uint thread_counter;
atomic_uint_fast64_t size_counter;

void clear_dir(const wstring& dir)
{
	list<thread*> threads;

	wstring dir_with_esc(dir);
	dir_with_esc += L"\\*";
	WIN32_FIND_DATA fd = { 0 };
	PVOID value;
	Wow64DisableWow64FsRedirection(&value);
	HANDLE file = FindFirstFile(dir_with_esc.c_str(), &fd);
	if (file != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (wcscmp(fd.cFileName, L".") == 0 ||
				wcscmp(fd.cFileName, L"..") == 0)
			{
				continue;
			}
			wstring child_dir(dir);
			child_dir += fd.cFileName;
			if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				child_dir += L"\\";
				threads.push_back(new thread(clear_dir, std::move(child_dir)));
				++thread_counter;
			}
			else
			{
				size_counter += fd.nFileSizeLow;
				DeleteFile(child_dir.c_str());
			}
		} while (FindNextFile(file, &fd));
	}
	Wow64RevertWow64FsRedirection(value);
	if (file)
	{
		FindClose(file);
	}

	for (auto& p : threads)
	{
		p->join();
		delete p;
	}
	RemoveDirectory(dir.c_str());
}

int main()
{
	cout << "Press Y to clear tmp files..." << std::endl;
	char clear_flag;
	cin >> clear_flag;
	if (clear_flag == 'y' || clear_flag == 'Y')
	{
		wchar_t temp_path[MAX_PATH] = { 0 };
		DWORD length = GetTempPath(MAX_PATH, temp_path);
		if (length == 0)
		{
			return 0;
		}
		thread worker(clear_dir, temp_path);
		++thread_counter;
		worker.join();
	}
	cout << "threads : " << thread_counter << endl;
	cout << "total size : " << size_counter << endl;
	cout << "Press any key to exit..." << std::endl;
	getchar();
	return 0;
}
