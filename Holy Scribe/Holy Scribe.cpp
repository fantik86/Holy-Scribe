#include <iostream>
#include <Windows.h>
#include <conio.h>
#include <chrono>
#include <thread>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>

std::pair<int, int> getConsoleWindowSize() {
	CONSOLE_SCREEN_BUFFER_INFO console_scr_buffer_info;
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &console_scr_buffer_info);

	SMALL_RECT wnd = console_scr_buffer_info.srWindow;

	return std::make_pair<int, int>(wnd.Bottom - wnd.Top + 1, wnd.Right - wnd.Left + 1);
}

static std::stringstream fileBuf;
static const int alignFromLeft = 7;
static std::pair<int, int> window_size = getConsoleWindowSize();

static std::vector<std::vector<wchar_t>> outBuffer;



std::string intToBible(int value) {
	std::string str;
	str += std::to_string((value - (value % 10)) / 10) + ":";
	str += std::to_string(value % 10);
	return str;
}

void Delay(int value) {
	std::this_thread::sleep_for(std::chrono::milliseconds(value));
}

void eraseChar(std::vector<std::vector<wchar_t>>* buf) {
	CONSOLE_SCREEN_BUFFER_INFO console_scr_buffer_info;
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &console_scr_buffer_info);


	console_scr_buffer_info.dwCursorPosition.X -= 1;
}

void processInput(std::vector<std::vector<wchar_t>>* buf) {
	while (true) {
		CONSOLE_SCREEN_BUFFER_INFO console_scr_buffer_info;
		GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &console_scr_buffer_info);

		int ch = getch();
		if (ch == 0 || ch == 224) {
			ch = 256 + getch();
		}
		if (ch == 256 + 72) { // UP
			console_scr_buffer_info.dwCursorPosition.Y -= 1;
		}
		if (ch == 256 + 75) { // LEFT
			if (console_scr_buffer_info.dwCursorPosition.X > alignFromLeft) {
				console_scr_buffer_info.dwCursorPosition.X -= 1;
			}
		}
		if (ch == 256 + 80) { // DOWN
			if (console_scr_buffer_info.dwCursorPosition.Y < getConsoleWindowSize().first - 1) {
				console_scr_buffer_info.dwCursorPosition.Y += 1;
			}
		}
		if (ch == 256 + 77) { // RIGHT
			console_scr_buffer_info.dwCursorPosition.X += 1;
		}
		if (ch == 8) { // BACKSPACE
			if (console_scr_buffer_info.dwCursorPosition.X > alignFromLeft) {
				(*buf).at(console_scr_buffer_info.dwCursorPosition.Y).at(console_scr_buffer_info.dwCursorPosition.X - 1) = ' ';
				console_scr_buffer_info.dwCursorPosition.X -= 1;
			}
			
		}
		SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), console_scr_buffer_info.dwCursorPosition);
	}
}


void loadFile(const std::string filename) {
	std::fstream fs;
	fs.open(filename, std::ios::in | std::ios::out);
	fileBuf << fs.rdbuf();
}

void copyStrToBuf(std::vector<std::vector<wchar_t>>& buf, int row, int column, const std::pair<int, int>& window_size, const std::string str) {
	column += alignFromLeft;
	for (char c : str) {
		if (column >= window_size.second) {
			++row;
			column = alignFromLeft;
		}
		switch (c) {
		case '\n':
			++row;
			column = alignFromLeft;
			break;
		default:
			buf.at(row).at(column) = c;
			++column;
			break;
		}
	}
}

void Update() {
	window_size = getConsoleWindowSize();

	DWORD charsWritten;

	for (int row = 0; row < window_size.first; ++row) {
		for (int col = 0; col < window_size.second; ++col) {
			if (col < alignFromLeft - 2) {
				outBuffer.at(row).at(col) = ' ';
			}

			copyStrToBuf(outBuffer, row, -6, window_size, intToBible(row));

			outBuffer.at(row).at(alignFromLeft - 2) = '|';

			WriteConsoleOutputCharacterW(GetStdHandle(STD_OUTPUT_HANDLE), &outBuffer.at(row).at(col), 1, COORD{ static_cast<short>(col), static_cast<short>(row) }, &charsWritten);
		}
	}


	Delay(1);
}

int main()
{


	
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), COORD{ alignFromLeft, 0 });



	outBuffer.resize(window_size.first);

	for (std::vector<wchar_t>& outBufferRow : outBuffer) {
		outBufferRow.resize(window_size.second);
	}


	for (int row = 0; row < window_size.first; ++row) {
		for (int column = 0; column < window_size.second; ++column) {
			outBuffer.at(row).at(column) = ' ';
		}
	}

	fileBuf << "AWRSWWWWWW!!!!!!!!!!!!!!!!!!!!!!!!\nTEEEST";

	copyStrToBuf(outBuffer, 0, 0, window_size, fileBuf.str());

	std::thread cursor_update_thread(processInput, &outBuffer);

	while (true) {
		Update();
	}
}

