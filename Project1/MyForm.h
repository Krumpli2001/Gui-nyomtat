#pragma once

#include <iostream>
#include <format>
#include <string>
#include <fstream>
#include <msclr/marshal_cppstd.h>
#include <windows.h>
#include <gdiplus.h>
#include <stdlib.h> // a wchar konverziohoz
#include <thread>
#include <chrono>
#include <array>
//#include <memory>

#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "comdlg32.lib")

int hatvany(int alap, int hatvany) {
	auto a = 1;
	for (int i = 0; i < hatvany; ++i) {
		a *= alap;
	}
	return a;
}

int find_nth_of(const std::string& str, const int n, const char ch) {
	int num = 0;
	for (auto i = 0; i < str.length(); ++i) {
		if (str[i] == ch) {
			++num;
		}
		if (num == n) {
			return i;
		}
	}
	return -1;
}

void fajlba_iras(int Fajta, int Ev, std::array<int, 12>* intervalumok, std::array<std::string, 6>* datumok, std::array<bool, 6>* irany) {
	std::ofstream o("settings.txt");
	auto port = "Manualisan ne ird felul - SA";
	if (o.is_open()) {
		o << port << '\n' << Fajta << '\n' << Ev << "\n,";
		for (const auto i : *intervalumok) {
			o << i << ',';
		}
		o << "\n,";
		for (const auto i : *datumok) {
			o << i << ',';
		}
		o << "\n,";
		for (const auto i : *irany) {
			o << i << ',';
		}
		o << '\n';
		//o << '\n' << Erkezett << '\n' << BeKi << '\n';
		o.close();
	}
}

auto to_wchar(const std::string& str) {
	auto newsize = str.length() + 1;
	auto wcstring = std::make_unique_for_overwrite<wchar_t[]>(newsize);
	size_t convertedChars = 0; //mindenkeppen size_t kell neki
	mbstowcs_s(&convertedChars, wcstring.get(), newsize, str.data(), _TRUNCATE);
	return wcstring;
}

std::tuple<int, int, int> strdatumTotuple(const std::string& datum) {
	auto YYYY = std::stoi(datum.substr(0, find_nth_of(datum, 1, '.')));
	auto MM = std::stoi(datum.substr(find_nth_of(datum, 1, '.') + 1, find_nth_of(datum, 2, '.') - find_nth_of(datum, 1, '.') - 1));
	auto DD = std::stoi(datum.substr(find_nth_of(datum, 2, '.') + 1, find_nth_of(datum, 3, '.') - find_nth_of(datum, 2, '.') - 1));
	return std::make_tuple(YYYY, MM, DD);
}

void nyomtatas(std::array<int, 12>* intervalumok, int K, int V, int Fajta, int EvI, std::array<std::string, 6>* datumok, int nyom, std::array<bool, 6>* irany) {
	auto Ev = std::to_string(EvI);
	auto F = 0;
	// Nyomtato kivalasztasa (az alapertelmezettre, ha nem akkor meg dobjon ablakot a PDFhez)
	PRINTDLG pd = { 0 };
	pd.lStructSize = sizeof(pd);
	//pd.Flags = PD_RETURNDC | PD_RETURNDEFAULT;
	pd.Flags = PD_RETURNDC;

	// No nyomtató
	if (nyom && !PrintDlgW(&pd)) {
		std::cout << ("Could not find default printer.\n");
		System::Windows::Forms::MessageBox::Show("Nem található az alapértelmezett nyomtató");
	}

	HDC hdc;
	//Nyomtato hDC
	if (nyom) {
		hdc = pd.hDC;
	}
	else {
		hdc = CreateDCW(L"WINSPOOL", L"Microsoft Print to PDF", NULL, NULL);
	}

	//kep letrehozasa, file nevvel
	Gdiplus::Image image(L"logo.jpg");
	if (image.GetLastStatus() == Gdiplus::Ok && hdc) {
		//Nyomtatasi job, doc meret, "file nev" a többi meg NULL
		DOCINFO di = { sizeof(DOCINFO), L"C++ Print Job" };

		//di.lpszOutput = L"C:\\Users\\Public\\Documents\\test_print.pdf";

		auto kezdet = K;
		auto veg = V;

		std::this_thread::sleep_for(std::chrono::milliseconds(500));

		//Nyomtatas megkezdese, hdc es docifo atadasa
		if (StartDocW(hdc, &di) > 0) {


			while (kezdet <= veg) {

				auto IntervalumK = std::to_string(kezdet);
				//auto tiz = 1;
				for (auto i = 0; i < 5; ++i) {
					if (kezdet < hatvany(10,i)) {
						IntervalumK = '0' + IntervalumK;
					}
				}

				//ezzel "megkezd egy uj papir lapot"
				if (StartPage(hdc) != 0) {

					HFONT hFont = CreateFontW(30, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, NONANTIALIASED_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Arial");
					HFONT hBoldFont = CreateFontW(50, 0, 0, 0, FW_BOLD, 0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, NONANTIALIASED_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Arial");

					auto hOldFont = (HFONT)SelectObject(hdc, hFont);
					DeleteObject(hOldFont);

					//a koordinatak elvileg pixelekben vannak
					//kep nyomtatasa + koordinatak
					Gdiplus::Graphics graphics(hdc);

					graphics.Clear(Gdiplus::Color::White);
					graphics.SetInterpolationMode(Gdiplus::InterpolationModeNearestNeighbor);
					graphics.SetPixelOffsetMode(Gdiplus::PixelOffsetModeHighQuality);
					graphics.SetSmoothingMode(Gdiplus::SmoothingModeNone);

					auto w = 16;
					auto h = 26;

					if (image.GetLastStatus() == Gdiplus::Ok) {
						if (nyom) {
							w = 50;
							h = 80;
							graphics.DrawImage(&image, 60, 0, w, h);
						}
						else {
							graphics.DrawImage(&image, 15, 0, w, h);
						}
					}

					std::cout << std::format("{} {}\n", w, h);

					auto MOBx = 110;
					auto MOBy = 165;

					if (nyom) {
						MOBx = 143;
					}

					const wchar_t* MOB = L"MOB";
					TextOutW(hdc, MOBx, MOBy, MOB, static_cast<int>(wcslen(MOB)));

					auto we = L"Érkezett: ";

					// a goto-k miatt nem switch-caseel van megoldva...

					if (Fajta == 0) {
						if ((*irany)[0]) {
							we = L"Kiküldve: ";
						}
						SelectObject(hdc, hBoldFont);
						auto mob = L"MOB-";
						TextOutW(hdc, 315, 20, mob, static_cast<int>(wcslen(mob)));
						auto faj = IntervalumK + '/' + Ev;
						auto wfaj = to_wchar(faj);
						TextOutW(hdc, 255, 80, wfaj.get(), static_cast<int>(wcslen(wfaj.get())));
						SelectObject(hdc, hFont);
						auto e = (*datumok)[0];
						auto seged = to_wchar(e);
						//auto we = L"Érkezett: ";
						auto newsize = wcslen(we) + wcslen(seged.get()) + 1;
						auto erk = std::make_unique<wchar_t[]>(newsize);
						wcsncat_s(erk.get(), newsize, we, wcslen(we));
						wcsncat_s(erk.get(), newsize, seged.get(), wcslen(seged.get()));
						TextOutW(hdc, 250, 150, erk.get(), static_cast<int>(wcslen(erk.get())));
						(*intervalumok)[0] = K;
						(*intervalumok)[1] = V;
					}

					else if (Fajta == 1) {
						if ((*irany)[1]) {
							we = L"Kiküldve: ";
						}
						SelectObject(hdc, hBoldFont);
						F = 1;
						auto szla = L"SZLA-";
						TextOutW(hdc, 300, 20, szla, static_cast<int>(wcslen(szla)));
						auto faj = IntervalumK + '/' + Ev;
						auto wfaj = to_wchar(faj);
						TextOutW(hdc, 255, 80, wfaj.get(), static_cast<int>(wcslen(wfaj.get())));
						SelectObject(hdc, hFont);
						auto e = (*datumok)[1];
						auto seged = to_wchar(e);
						//auto we = L"Érkezett: ";
						auto newsize = wcslen(we) + wcslen(seged.get()) + 1;
						auto erk = std::make_unique<wchar_t[]>(newsize);
						wcsncat_s(erk.get(), newsize, we, wcslen(we));
						wcsncat_s(erk.get(), newsize, seged.get(), wcslen(seged.get()));
						TextOutW(hdc, 250, 150, erk.get(), static_cast<int>(wcslen(erk.get())));
						(*intervalumok)[2] = K;
						(*intervalumok)[3] = V;
					}

					else if (Fajta == 2) {
						if ((*irany)[2]) {
							we = L"Kiküldve: ";
						}
						SelectObject(hdc, hBoldFont);
						F = 2;
						auto mobdok = L"MOB-DOK-";
						TextOutW(hdc, 255, 20, mobdok, static_cast<int>(wcslen(mobdok)));
						auto faj = IntervalumK + '/' + Ev;
						auto wfaj = to_wchar(faj);
						TextOutW(hdc, 255, 80, wfaj.get(), static_cast<int>(wcslen(wfaj.get())));
						SelectObject(hdc, hFont);
						auto e = (*datumok)[2];
						auto seged = to_wchar(e);
						//auto we = L"Érkezett: ";
						auto newsize = wcslen(we) + wcslen(seged.get()) + 1;
						auto erk = std::make_unique<wchar_t[]>(newsize);
						wcsncat_s(erk.get(), newsize, we, wcslen(we));
						wcsncat_s(erk.get(), newsize, seged.get(), wcslen(seged.get()));
						TextOutW(hdc, 250, 150, erk.get(), static_cast<int>(wcslen(erk.get())));
						(*intervalumok)[4] = K;
						(*intervalumok)[5] = V;
					}

					else if (Fajta == 3) {
						if ((*irany)[3]) {
							we = L"Kiküldve: ";
						}
						SelectObject(hdc, hBoldFont);
						F = 3;
						auto mobdok = L"SÁVB-";
						TextOutW(hdc, 300, 20, mobdok, static_cast<int>(wcslen(mobdok)));
						auto faj = IntervalumK + '/' + Ev;
						auto wfaj = to_wchar(faj);
						TextOutW(hdc, 255, 80, wfaj.get(), static_cast<int>(wcslen(wfaj.get())));
						SelectObject(hdc, hFont);
						auto e = (*datumok)[3];
						auto seged = to_wchar(e);
						//auto we = L"Érkezett: ";
						auto newsize = wcslen(we) + wcslen(seged.get()) + 1;
						auto erk = std::make_unique<wchar_t[]>(newsize);
						wcsncat_s(erk.get(), newsize, we, wcslen(we));
						wcsncat_s(erk.get(), newsize, seged.get(), wcslen(seged.get()));
						TextOutW(hdc, 250, 150, erk.get(), static_cast<int>(wcslen(erk.get())));
						(*intervalumok)[6] = K;
						(*intervalumok)[7] = V;
					}

					else if (Fajta == 4) {
						if ((*irany)[4]) {
							we = L"Kiküldve: ";
						}
						SelectObject(hdc, hBoldFont);
						F = 4;
						auto mobdok = L"MOB-2K-";
						TextOutW(hdc, 280, 20, mobdok, static_cast<int>(wcslen(mobdok)));
						auto faj = IntervalumK + '/' + Ev;
						auto wfaj = to_wchar(faj);
						TextOutW(hdc, 255, 80, wfaj.get(), static_cast<int>(wcslen(wfaj.get())));
						SelectObject(hdc, hFont);
						auto e = (*datumok)[4];
						auto seged = to_wchar(e);
						//auto we = L"Érkezett: ";
						auto newsize = wcslen(we) + wcslen(seged.get()) + 1;
						auto erk = std::make_unique<wchar_t[]>(newsize);
						wcsncat_s(erk.get(), newsize, we, wcslen(we));
						wcsncat_s(erk.get(), newsize, seged.get(), wcslen(seged.get()));
						TextOutW(hdc, 250, 150, erk.get(), static_cast<int>(wcslen(erk.get())));
						(*intervalumok)[8] = K;
						(*intervalumok)[9] = V;
					}

					else if (Fajta == 5) {
						if ((*irany)[5]) {
							we = L"Kiküldve: ";
						}
						SelectObject(hdc, hBoldFont);
						F = 5;
						auto mobdok = L"MOB-BÉR-";
						TextOutW(hdc, 260, 20, mobdok, static_cast<int>(wcslen(mobdok)));
						auto faj = IntervalumK + '/' + Ev;
						auto wfaj = to_wchar(faj);
						TextOutW(hdc, 255, 80, wfaj.get(), static_cast<int>(wcslen(wfaj.get())));
						SelectObject(hdc, hFont);
						auto e = (*datumok)[5];
						auto seged = to_wchar(e);
						//auto we = L"Érkezett: ";
						auto newsize = wcslen(we) + wcslen(seged.get()) + 1;
						auto erk = std::make_unique<wchar_t[]>(newsize);
						wcsncat_s(erk.get(), newsize, we, wcslen(we));
						wcsncat_s(erk.get(), newsize, seged.get(), wcslen(seged.get()));
						TextOutW(hdc, 250, 150, erk.get(), static_cast<int>(wcslen(erk.get())));
						(*intervalumok)[10] = K;
						(*intervalumok)[11] = V;
					}

					//SelectObject(hdc, hOldFont);
					//DeleteObject(hFont);
					DeleteObject(hBoldFont);

					EndPage(hdc);
				}
				++kezdet;
			}
			// Vegeztunk
			EndDoc(hdc);
			std::cout << ("Print job sent!");
		}

	}
	else {
		std::cout << ("Could not find image.\n");
		System::Windows::Forms::MessageBox::Show("Az alapértelmezett kép nem található");
	}

	if (hdc) {
		DeleteDC(hdc);
	}

	//if(Fajta=="MOB")

	fajlba_iras(F, EvI, intervalumok, datumok, irany);
}

namespace Project1 {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;

	/// <summary>
	/// Summary for MyForm
	/// </summary>
	public ref class MyForm : public System::Windows::Forms::Form
	{
	public:
		MyForm(void)
		{
			InitializeComponent();

			auto port = std::string("Manualisan ne ird felul - SA");
			auto Fajta = std::string("0");
			auto Ev = std::string("2026");
			auto interstring = std::string("");

			datumok = new std::array<std::string, 6>();
			for (int i = 0; i < 6; ++i) {
				(*datumok)[i] = "2026.3.02";
			}

			//std::array<int, 12> intervalumok{};
			intervalumok = new std::array<int, 12>();
			prevK = new int;
			*prevK = 0;
			for (int i = 0; i < 12; ++i) {
				//intervalumok->at(i) = 0;
				(*intervalumok)[i] = 0;
			}

			irany = new std::array<bool, 6>();
			for (int i = 0; i < 6; ++i) {
				(*irany)[i] = false;
			}

			std::string line;
			std::string conf;
			std::ifstream settings("settings.txt");
			if (settings.is_open()) {
				while (std::getline(settings, line)) {
					conf += line + '\n';
				}
				settings.close();

				port = conf.substr(0, find_nth_of(conf, 1, '\n'));
				Fajta = conf.substr(find_nth_of(conf, 1, '\n') + 1, find_nth_of(conf, 2, '\n') - find_nth_of(conf, 1, '\n') - 1);
				Ev = conf.substr(find_nth_of(conf, 2, '\n') + 1, find_nth_of(conf, 3, '\n') - find_nth_of(conf, 2, '\n') - 1);
				/*IntervalumK = conf.substr(find_nth_of(conf, 3, '\n') + 1, find_nth_of(conf, 4, '\n') - find_nth_of(conf, 3, '\n') - 1);
				IntervalumV = conf.substr(find_nth_of(conf, 4, '\n') + 1, find_nth_of(conf, 5, '\n') - find_nth_of(conf, 4, '\n') - 1);*/

				interstring = conf.substr(find_nth_of(conf, 3, '\n') + 1, find_nth_of(conf, 4, '\n') - find_nth_of(conf, 3, '\n') - 1);

				for (int i = 0; i < 12; ++i) {
					(*intervalumok)[i] = std::stoi(interstring.substr(find_nth_of(interstring, i + 1, ',') + 1, find_nth_of(interstring, i + 2, ',') - find_nth_of(interstring, i + 1, ',') - 1));
				}

				interstring = conf.substr(find_nth_of(conf, 4, '\n') + 1, find_nth_of(conf, 5, '\n') - find_nth_of(conf, 4, '\n') - 1);

				for (int i = 0; i < 6; ++i) {
					(*datumok)[i] = interstring.substr(find_nth_of(interstring, i + 1, ',') + 1, find_nth_of(interstring, i + 2, ',') - find_nth_of(interstring, i + 1, ',') - 1);
				}

				interstring = conf.substr(find_nth_of(conf, 5, '\n') + 1, find_nth_of(conf, 6, '\n') - find_nth_of(conf, 5, '\n') - 1);

				for (int i = 0; i < 6; ++i) {
					(*irany)[i] = std::stoi(interstring.substr(find_nth_of(interstring, i + 1, ',') + 1, find_nth_of(interstring, i + 2, ',') - find_nth_of(interstring, i + 1, ',') - 1));
				}

				/*Erkezett = conf.substr(find_nth_of(conf, 4, '\n') + 1, find_nth_of(conf, 5, '\n') - find_nth_of(conf, 4, '\n') - 1);
				BeKi = conf.substr(find_nth_of(conf, 5, '\n') + 1, find_nth_of(conf, 6, '\n') - find_nth_of(conf, 5, '\n') - 1);*/
			}

			this->comboBox1->SelectedIndex = std::stoi(Fajta);
			this->numericUpDown1->Value = std::stoi(Ev);

			auto IntervalumK = 0;
			auto IntervalumV = 0;

			std::string Erkezett = "";
			bool BeKi = false;

			switch (std::stoi(Fajta)) {
			case 0:
				IntervalumK = (*intervalumok)[0];
				IntervalumV = (*intervalumok)[1];
				Erkezett = (*datumok)[0];
				BeKi = (*irany)[0];
				break;
			case 1:
				IntervalumK = (*intervalumok)[2];
				IntervalumV = (*intervalumok)[3];
				Erkezett = (*datumok)[1];
				BeKi = (*irany)[1];
				break;
			case 2:
				IntervalumK = (*intervalumok)[4];
				IntervalumV = (*intervalumok)[5];
				Erkezett = (*datumok)[2];
				BeKi = (*irany)[2];
				break;
			case 3:
				IntervalumK = (*intervalumok)[6];
				IntervalumV = (*intervalumok)[7];
				Erkezett = (*datumok)[3];
				BeKi = (*irany)[3];
				break;
			case 4:
				IntervalumK = (*intervalumok)[8];
				IntervalumV = (*intervalumok)[9];
				Erkezett = (*datumok)[4];
				BeKi = (*irany)[4];
				break;
			case 5:
				IntervalumK = (*intervalumok)[10];
				IntervalumV = (*intervalumok)[11];
				Erkezett = (*datumok)[5];
				BeKi = (*irany)[5];
				break;
			}

			this->numericUpDown2->Value = IntervalumK;
			this->numericUpDown3->Value = IntervalumV;

			this->comboBox2->SelectedIndex = BeKi;

			//kep nyomtatas inicializalasa 15*240
			Gdiplus::GdiplusStartupInput gdiplusStartupInput;
			auto asd = this->gdiplusToken;
			Gdiplus::GdiplusStartup(&asd, &gdiplusStartupInput, NULL);
			//auto asd = this->gdiplusToken;
			*prevK = IntervalumK;
		}

	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~MyForm()
		{
			if (components)
			{
				delete components;
			}
			auto asd = this->gdiplusToken;
			Gdiplus::GdiplusShutdown(asd);
			//csak egyszer van meghivva a new, nem okoz leket, crashel ha megprobalom deletelni, 
			// valoszinuleg ezek is component-nek szamitanak
			//delete intervalumok;
			//delete prevK;
		}

	protected:
		ULONG_PTR gdiplusToken;
	private: System::Windows::Forms::Button^ button1;




	private: System::Windows::Forms::NumericUpDown^ numericUpDown1;
	private: System::Windows::Forms::NumericUpDown^ numericUpDown2;




	public: System::Windows::Forms::ComboBox^ comboBox1;
	private: System::Windows::Forms::NumericUpDown^ numericUpDown3;
	private: System::Windows::Forms::DateTimePicker^ dateTimePicker1;
	private: System::Windows::Forms::Button^ button2;
	private: System::Windows::Forms::TextBox^ textBox1;
	private: System::Windows::Forms::TextBox^ textBox2;
	private: System::Windows::Forms::TextBox^ textBox3;
	private: System::Windows::Forms::TextBox^ textBox4;
	private: System::Windows::Forms::SaveFileDialog^ saveFileDialog1;
	public: System::Windows::Forms::ComboBox^ comboBox2;
	private:
	private: System::Windows::Forms::TextBox^ textBox5;
	public:

	protected:

	private:
		std::array<int, 12>* intervalumok;
		int* prevK;
		std::array<std::string, 6>* datumok;
		std::array<bool, 6>* irany;
		/// <summary>
		/// Required designer variable.
		/// </summary>
		System::ComponentModel::Container^ components;

#pragma region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
			this->button1 = (gcnew System::Windows::Forms::Button());
			this->numericUpDown1 = (gcnew System::Windows::Forms::NumericUpDown());
			this->numericUpDown2 = (gcnew System::Windows::Forms::NumericUpDown());
			this->comboBox1 = (gcnew System::Windows::Forms::ComboBox());
			this->numericUpDown3 = (gcnew System::Windows::Forms::NumericUpDown());
			this->dateTimePicker1 = (gcnew System::Windows::Forms::DateTimePicker());
			this->button2 = (gcnew System::Windows::Forms::Button());
			this->textBox1 = (gcnew System::Windows::Forms::TextBox());
			this->textBox2 = (gcnew System::Windows::Forms::TextBox());
			this->textBox3 = (gcnew System::Windows::Forms::TextBox());
			this->textBox4 = (gcnew System::Windows::Forms::TextBox());
			this->saveFileDialog1 = (gcnew System::Windows::Forms::SaveFileDialog());
			this->comboBox2 = (gcnew System::Windows::Forms::ComboBox());
			this->textBox5 = (gcnew System::Windows::Forms::TextBox());
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numericUpDown1))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numericUpDown2))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numericUpDown3))->BeginInit();
			this->SuspendLayout();
			// 
			// button1
			// 
			this->button1->Location = System::Drawing::Point(16, 358);
			this->button1->Margin = System::Windows::Forms::Padding(4);
			this->button1->Name = L"button1";
			this->button1->Size = System::Drawing::Size(88, 54);
			this->button1->TabIndex = 0;
			this->button1->Text = L"Nyomtatás";
			this->button1->UseVisualStyleBackColor = true;
			this->button1->Click += gcnew System::EventHandler(this, &MyForm::button1_Click);
			// 
			// numericUpDown1
			// 
			this->numericUpDown1->Location = System::Drawing::Point(168, 48);
			this->numericUpDown1->Margin = System::Windows::Forms::Padding(4);
			this->numericUpDown1->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 9999, 0, 0, 0 });
			this->numericUpDown1->Minimum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 1970, 0, 0, 0 });
			this->numericUpDown1->Name = L"numericUpDown1";
			this->numericUpDown1->Size = System::Drawing::Size(160, 22);
			this->numericUpDown1->TabIndex = 5;
			this->numericUpDown1->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) { 2026, 0, 0, 0 });
			this->numericUpDown1->ValueChanged += gcnew System::EventHandler(this, &MyForm::numericUpDown1_ValueChanged);
			// 
			// numericUpDown2
			// 
			this->numericUpDown2->Location = System::Drawing::Point(168, 134);
			this->numericUpDown2->Margin = System::Windows::Forms::Padding(4);
			this->numericUpDown2->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 99999, 0, 0, 0 });
			this->numericUpDown2->Name = L"numericUpDown2";
			this->numericUpDown2->Size = System::Drawing::Size(160, 22);
			this->numericUpDown2->TabIndex = 6;
			this->numericUpDown2->ValueChanged += gcnew System::EventHandler(this, &MyForm::numericUpDown2_ValueChanged);
			// 
			// comboBox1
			// 
			this->comboBox1->FormattingEnabled = true;
			this->comboBox1->Items->AddRange(gcnew cli::array< System::Object^  >(6) {
				L"MOB", L"SZLA", L"MOB-DOK", L"SÁVB", L"MOB-2K",
					L"MOB-BÉR"
			});
			this->comboBox1->Location = System::Drawing::Point(168, 15);
			this->comboBox1->Margin = System::Windows::Forms::Padding(4);
			this->comboBox1->Name = L"comboBox1";
			this->comboBox1->Size = System::Drawing::Size(160, 24);
			this->comboBox1->TabIndex = 11;
			this->comboBox1->SelectedIndexChanged += gcnew System::EventHandler(this, &MyForm::comboBox1_SelectedIndexChanged);
			// 
			// numericUpDown3
			// 
			this->numericUpDown3->Location = System::Drawing::Point(168, 177);
			this->numericUpDown3->Margin = System::Windows::Forms::Padding(4);
			this->numericUpDown3->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 99999, 0, 0, 0 });
			this->numericUpDown3->Name = L"numericUpDown3";
			this->numericUpDown3->Size = System::Drawing::Size(160, 22);
			this->numericUpDown3->TabIndex = 12;
			this->numericUpDown3->ValueChanged += gcnew System::EventHandler(this, &MyForm::numericUpDown3_ValueChanged);
			// 
			// dateTimePicker1
			// 
			this->dateTimePicker1->Format = System::Windows::Forms::DateTimePickerFormat::Short;
			this->dateTimePicker1->Location = System::Drawing::Point(168, 272);
			this->dateTimePicker1->Margin = System::Windows::Forms::Padding(4);
			this->dateTimePicker1->Name = L"dateTimePicker1";
			this->dateTimePicker1->Size = System::Drawing::Size(157, 22);
			this->dateTimePicker1->TabIndex = 13;
			// 
			// button2
			// 
			this->button2->Location = System::Drawing::Point(241, 358);
			this->button2->Margin = System::Windows::Forms::Padding(4);
			this->button2->Name = L"button2";
			this->button2->Size = System::Drawing::Size(88, 54);
			this->button2->TabIndex = 14;
			this->button2->Text = L"PDF Generálás";
			this->button2->UseVisualStyleBackColor = true;
			this->button2->Click += gcnew System::EventHandler(this, &MyForm::button2_Click);
			// 
			// textBox1
			// 
			this->textBox1->BackColor = System::Drawing::SystemColors::Control;
			this->textBox1->BorderStyle = System::Windows::Forms::BorderStyle::None;
			this->textBox1->Location = System::Drawing::Point(120, 18);
			this->textBox1->Margin = System::Windows::Forms::Padding(4);
			this->textBox1->Name = L"textBox1";
			this->textBox1->Size = System::Drawing::Size(40, 15);
			this->textBox1->TabIndex = 15;
			this->textBox1->Text = L"Fajta:";
			this->textBox1->TextChanged += gcnew System::EventHandler(this, &MyForm::textBox1_TextChanged);
			// 
			// textBox2
			// 
			this->textBox2->BackColor = System::Drawing::SystemColors::Control;
			this->textBox2->BorderStyle = System::Windows::Forms::BorderStyle::None;
			this->textBox2->Location = System::Drawing::Point(120, 50);
			this->textBox2->Margin = System::Windows::Forms::Padding(4);
			this->textBox2->Name = L"textBox2";
			this->textBox2->Size = System::Drawing::Size(33, 15);
			this->textBox2->TabIndex = 16;
			this->textBox2->Text = L"Év:";
			// 
			// textBox3
			// 
			this->textBox3->BackColor = System::Drawing::SystemColors::Control;
			this->textBox3->BorderStyle = System::Windows::Forms::BorderStyle::None;
			this->textBox3->Location = System::Drawing::Point(23, 137);
			this->textBox3->Margin = System::Windows::Forms::Padding(4);
			this->textBox3->Multiline = true;
			this->textBox3->Name = L"textBox3";
			this->textBox3->Size = System::Drawing::Size(137, 26);
			this->textBox3->TabIndex = 17;
			this->textBox3->Text = L"Intervalum kezdete:";
			// 
			// textBox4
			// 
			this->textBox4->BackColor = System::Drawing::SystemColors::Control;
			this->textBox4->BorderStyle = System::Windows::Forms::BorderStyle::None;
			this->textBox4->Location = System::Drawing::Point(23, 180);
			this->textBox4->Margin = System::Windows::Forms::Padding(4);
			this->textBox4->Multiline = true;
			this->textBox4->Name = L"textBox4";
			this->textBox4->Size = System::Drawing::Size(119, 26);
			this->textBox4->TabIndex = 18;
			this->textBox4->Text = L"Intervalum vége:";
			this->textBox4->TextChanged += gcnew System::EventHandler(this, &MyForm::textBox4_TextChanged);
			// 
			// comboBox2
			// 
			this->comboBox2->FormattingEnabled = true;
			this->comboBox2->Items->AddRange(gcnew cli::array< System::Object^  >(2) { L"Érkezett", L"Kiküldve" });
			this->comboBox2->Location = System::Drawing::Point(168, 240);
			this->comboBox2->Margin = System::Windows::Forms::Padding(4);
			this->comboBox2->Name = L"comboBox2";
			this->comboBox2->Size = System::Drawing::Size(160, 24);
			this->comboBox2->TabIndex = 19;
			// 
			// textBox5
			// 
			this->textBox5->BackColor = System::Drawing::SystemColors::Control;
			this->textBox5->BorderStyle = System::Windows::Forms::BorderStyle::None;
			this->textBox5->Location = System::Drawing::Point(39, 240);
			this->textBox5->Margin = System::Windows::Forms::Padding(4);
			this->textBox5->Name = L"textBox5";
			this->textBox5->Size = System::Drawing::Size(114, 15);
			this->textBox5->TabIndex = 20;
			this->textBox5->Text = L"Érkezett/Kikülve:";
			this->textBox5->TextChanged += gcnew System::EventHandler(this, &MyForm::textBox5_TextChanged);
			// 
			// MyForm
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(8, 16);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->BackColor = System::Drawing::SystemColors::Control;
			this->ClientSize = System::Drawing::Size(345, 427);
			this->Controls->Add(this->textBox5);
			this->Controls->Add(this->comboBox2);
			this->Controls->Add(this->textBox4);
			this->Controls->Add(this->textBox3);
			this->Controls->Add(this->textBox2);
			this->Controls->Add(this->textBox1);
			this->Controls->Add(this->button2);
			this->Controls->Add(this->dateTimePicker1);
			this->Controls->Add(this->numericUpDown3);
			this->Controls->Add(this->comboBox1);
			this->Controls->Add(this->numericUpDown2);
			this->Controls->Add(this->numericUpDown1);
			this->Controls->Add(this->button1);
			this->Margin = System::Windows::Forms::Padding(4);
			this->Name = L"MOB Címke";
			this->Text = L"MOB Címke";
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numericUpDown1))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numericUpDown2))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numericUpDown3))->EndInit();
			this->ResumeLayout(false);
			this->PerformLayout();

		}
#pragma endregion
	private: System::Void button1_Click(System::Object^ sender, System::EventArgs^ e) {

		/*std::string finalPath;
		if (saveFileDialog1->ShowDialog() == System::Windows::Forms::DialogResult::OK) {
			finalPath = msclr::interop::marshal_as<std::string>(saveFileDialog1->FileName);
		}*/

		/*auto fajta = comboBox1->SelectedItem->ToString();
		auto cFajta = msclr::interop::marshal_as<std::string>(fajta);*/
		auto cFajta = comboBox1->SelectedIndex;
		auto Ev = System::Decimal::ToInt32(numericUpDown1->Value);
		auto K = System::Decimal::ToInt32(numericUpDown2->Value);
		auto V = System::Decimal::ToInt32(numericUpDown3->Value);
		auto YYYY = dateTimePicker1->Value.Year;
		auto MM = dateTimePicker1->Value.Month;
		auto DD = dateTimePicker1->Value.Day;
		auto BeKi = comboBox2->SelectedIndex;

		//std::cout << std::format("Nyomtat gomb lenyomva.\nErtekek: {} {} {} {} {}.{}.{}\n", cFajta, Ev, K, V, YYYY, MM, DD);

		auto honap = std::to_string(MM);
		auto nap = std::to_string(DD);

		if (MM < 10) {
			honap = std::format("0{}", honap);
		}
		if (DD < 10) {
			nap = std::format("0{}", nap);
		}

		(*datumok)[cFajta] = std::format("{}.{}.{}", YYYY, honap, nap);
		(*irany)[cFajta] = BeKi ? 1 : 0;

		nyomtatas(intervalumok, K, V, cFajta, Ev, datumok, 1, irany);

	}
	private: System::Void listBox1_SelectedIndexChanged(System::Object^ sender, System::EventArgs^ e) {
	}
	private: System::Void numericUpDown1_ValueChanged(System::Object^ sender, System::EventArgs^ e) {
	}
	private: System::Void numericUpDown2_ValueChanged(System::Object^ sender, System::EventArgs^ e) {

		auto K = System::Decimal::ToInt32(numericUpDown2->Value);
		//std::cout << std::format("{} {}\n", K, *prevK);
		auto kulonbseg = K - *prevK;
		std::cout << std::format("{}\n", kulonbseg);
		*prevK = K;
		std::tuple<int, int, int> t;

		numericUpDown3->Value = System::Decimal::ToInt32(numericUpDown3->Value) + kulonbseg;

		dateTimePicker1->Value = DateTime().Today;
		comboBox2->SelectedIndex = 0;

		switch (comboBox1->SelectedIndex) {
		case 0:
			if (System::Decimal::ToInt32(numericUpDown2->Value) == (*intervalumok)[0] && System::Decimal::ToInt32(numericUpDown3->Value) == (*intervalumok)[1]) {
				comboBox2->SelectedIndex = (*irany)[0];
				t = strdatumTotuple((*datumok)[0]);
				dateTimePicker1->Value = DateTime(std::get<0>(t), std::get<1>(t), std::get<2>(t));
			}
			break;
		case 1:
			if (System::Decimal::ToInt32(numericUpDown2->Value) == (*intervalumok)[2] && System::Decimal::ToInt32(numericUpDown3->Value) == (*intervalumok)[3]) {
				comboBox2->SelectedIndex = (*irany)[1];
				t = strdatumTotuple((*datumok)[1]);
				dateTimePicker1->Value = DateTime(std::get<0>(t), std::get<1>(t), std::get<2>(t));
			}
			break;
		case 2:
			if (System::Decimal::ToInt32(numericUpDown2->Value) == (*intervalumok)[4] && System::Decimal::ToInt32(numericUpDown3->Value) == (*intervalumok)[5]) {
				comboBox2->SelectedIndex = (*irany)[2];
				t = strdatumTotuple((*datumok)[2]);
				dateTimePicker1->Value = DateTime(std::get<0>(t), std::get<1>(t), std::get<2>(t));
			}
			break;
		case 3:
			if (System::Decimal::ToInt32(numericUpDown2->Value) == (*intervalumok)[6] && System::Decimal::ToInt32(numericUpDown3->Value) == (*intervalumok)[7]) {
				comboBox2->SelectedIndex = (*irany)[3];
				t = strdatumTotuple((*datumok)[3]);
				dateTimePicker1->Value = DateTime(std::get<0>(t), std::get<1>(t), std::get<2>(t));
			}
			break;
		case 4:
			if (System::Decimal::ToInt32(numericUpDown2->Value) == (*intervalumok)[8] && System::Decimal::ToInt32(numericUpDown3->Value) == (*intervalumok)[9]) {
				comboBox2->SelectedIndex = (*irany)[4];
				t = strdatumTotuple((*datumok)[4]);
				dateTimePicker1->Value = DateTime(std::get<0>(t), std::get<1>(t), std::get<2>(t));
			}
			break;
		case 5:
			if (System::Decimal::ToInt32(numericUpDown2->Value) == (*intervalumok)[10] && System::Decimal::ToInt32(numericUpDown3->Value) == (*intervalumok)[11]) {
				comboBox2->SelectedIndex = (*irany)[5];
				t = strdatumTotuple((*datumok)[5]);
				dateTimePicker1->Value = DateTime(std::get<0>(t), std::get<1>(t), std::get<2>(t));
			}
			break;
		}

	}
	private: System::Void dateTimePicker1_ValueChanged(System::Object^ sender, System::EventArgs^ e) {
	}
	private: System::Void listView1_SelectedIndexChanged(System::Object^ sender, System::EventArgs^ e) {
	}
	private: System::Void comboBox1_SelectedIndexChanged(System::Object^ sender, System::EventArgs^ e) {

		auto YYYY = 0;
		auto MM = 0;
		auto DD = 0;

		std::tuple<int, int, int> t;

		switch (comboBox1->SelectedIndex) {
		case 0:
			numericUpDown2->Value = (*intervalumok)[0];
			numericUpDown3->Value = (*intervalumok)[1];
			t = strdatumTotuple((*datumok)[0]);
			dateTimePicker1->Value = DateTime(std::get<0>(t), std::get<1>(t), std::get<2>(t));
			comboBox2->SelectedIndex = (*irany)[0];
			break;
		case 1:
			numericUpDown2->Value = (*intervalumok)[2];
			numericUpDown3->Value = (*intervalumok)[3];
			t = strdatumTotuple((*datumok)[1]);
			dateTimePicker1->Value = DateTime(std::get<0>(t), std::get<1>(t), std::get<2>(t));
			comboBox2->SelectedIndex = (*irany)[1];
			break;
		case 2:
			numericUpDown2->Value = (*intervalumok)[4];
			numericUpDown3->Value = (*intervalumok)[5];
			t = strdatumTotuple((*datumok)[2]);
			dateTimePicker1->Value = DateTime(std::get<0>(t), std::get<1>(t), std::get<2>(t));
			comboBox2->SelectedIndex = (*irany)[2];
			break;
		case 3:
			numericUpDown2->Value = (*intervalumok)[6];
			numericUpDown3->Value = (*intervalumok)[7];
			t = strdatumTotuple((*datumok)[3]);
			dateTimePicker1->Value = DateTime(std::get<0>(t), std::get<1>(t), std::get<2>(t));
			comboBox2->SelectedIndex = (*irany)[3];
			break;
		case 4:
			numericUpDown2->Value = (*intervalumok)[8];
			numericUpDown3->Value = (*intervalumok)[9];
			t = strdatumTotuple((*datumok)[4]);
			dateTimePicker1->Value = DateTime(std::get<0>(t), std::get<1>(t), std::get<2>(t));
			comboBox2->SelectedIndex = (*irany)[4];
			break;
		case 5:
			numericUpDown2->Value = (*intervalumok)[10];
			numericUpDown3->Value = (*intervalumok)[11];
			t = strdatumTotuple((*datumok)[5]);
			dateTimePicker1->Value = DateTime(std::get<0>(t), std::get<1>(t), std::get<2>(t));
			comboBox2->SelectedIndex = (*irany)[5];
			break;
		}
	}
	private: System::Void numericUpDown3_ValueChanged(System::Object^ sender, System::EventArgs^ e) {
		dateTimePicker1->Value = DateTime().Today;
		comboBox2->SelectedIndex = 0;

		std::tuple<int, int, int> t;

		switch (comboBox1->SelectedIndex) {
		case 0:
			if (System::Decimal::ToInt32(numericUpDown2->Value) == (*intervalumok)[0] && System::Decimal::ToInt32(numericUpDown3->Value) == (*intervalumok)[1]) {
				comboBox2->SelectedIndex = (*irany)[0];
				t = strdatumTotuple((*datumok)[0]);
				dateTimePicker1->Value = DateTime(std::get<0>(t), std::get<1>(t), std::get<2>(t));
			}
			break;
		case 1:
			if (System::Decimal::ToInt32(numericUpDown2->Value) == (*intervalumok)[2] && System::Decimal::ToInt32(numericUpDown3->Value) == (*intervalumok)[3]) {
				comboBox2->SelectedIndex = (*irany)[1];
				t = strdatumTotuple((*datumok)[1]);
				dateTimePicker1->Value = DateTime(std::get<0>(t), std::get<1>(t), std::get<2>(t));
			}
			break;
		case 2:
			if (System::Decimal::ToInt32(numericUpDown2->Value) == (*intervalumok)[4] && System::Decimal::ToInt32(numericUpDown3->Value) == (*intervalumok)[5]) {
				comboBox2->SelectedIndex = (*irany)[2];
				t = strdatumTotuple((*datumok)[2]);
				dateTimePicker1->Value = DateTime(std::get<0>(t), std::get<1>(t), std::get<2>(t));
			}
			break;
		case 3:
			if (System::Decimal::ToInt32(numericUpDown2->Value) == (*intervalumok)[6] && System::Decimal::ToInt32(numericUpDown3->Value) == (*intervalumok)[7]) {
				comboBox2->SelectedIndex = (*irany)[3];
				t = strdatumTotuple((*datumok)[3]);
				dateTimePicker1->Value = DateTime(std::get<0>(t), std::get<1>(t), std::get<2>(t));
			}
			break;
		case 4:
			if (System::Decimal::ToInt32(numericUpDown2->Value) == (*intervalumok)[8] && System::Decimal::ToInt32(numericUpDown3->Value) == (*intervalumok)[9]) {
				comboBox2->SelectedIndex = (*irany)[4];
				t = strdatumTotuple((*datumok)[4]);
				dateTimePicker1->Value = DateTime(std::get<0>(t), std::get<1>(t), std::get<2>(t));
			}
			break;
		case 5:
			if (System::Decimal::ToInt32(numericUpDown2->Value) == (*intervalumok)[10] && System::Decimal::ToInt32(numericUpDown3->Value) == (*intervalumok)[11]) {
				comboBox2->SelectedIndex = (*irany)[5];
				t = strdatumTotuple((*datumok)[5]);
				dateTimePicker1->Value = DateTime(std::get<0>(t), std::get<1>(t), std::get<2>(t));
			}
			break;
		}
	}
	private: System::Void dateTimePicker2_ValueChanged(System::Object^ sender, System::EventArgs^ e) {
	}
	private: System::Void button2_Click(System::Object^ sender, System::EventArgs^ e) {
		//auto fajta = comboBox1->SelectedItem->ToString();
		//auto cFajta = msclr::interop::marshal_as<std::string>(fajta);
		auto cFajta = comboBox1->SelectedIndex;
		auto Ev = System::Decimal::ToInt32(numericUpDown1->Value);
		auto K = System::Decimal::ToInt32(numericUpDown2->Value);
		auto V = System::Decimal::ToInt32(numericUpDown3->Value);
		auto YYYY = dateTimePicker1->Value.Year;
		auto MM = dateTimePicker1->Value.Month;
		auto DD = dateTimePicker1->Value.Day;
		auto BeKi = comboBox2->SelectedIndex;

		//std::cout << std::format("Nyomtat gomb lenyomva.\nErtekek: {} {} {} {} {}.{}.{}\n", cFajta, Ev, K, V, YYYY, MM, DD);

		auto honap = std::to_string(MM);
		auto nap = std::to_string(DD);

		if (MM < 10) {
			honap = std::format("0{}", honap);
		}
		if (DD < 10) {
			nap = std::format("0{}", nap);
		}

		(*datumok)[cFajta] = std::format("{}.{}.{}", YYYY, honap, nap);
		(*irany)[cFajta] = BeKi ? 1 : 0;

		nyomtatas(intervalumok, K, V, cFajta, Ev, datumok, 0, irany);

		//this->Close();
	}
	private: System::Void textBox1_TextChanged(System::Object^ sender, System::EventArgs^ e) {
	}
	private: System::Void textBox4_TextChanged(System::Object^ sender, System::EventArgs^ e) {
	}
	private: System::Void textBox5_TextChanged(System::Object^ sender, System::EventArgs^ e) {
	}
};
}
