#include "MyForm.h"

using namespace System;
using namespace System::Windows::Forms;

int main() {

	System::Windows::Forms::Application::EnableVisualStyles();
	System::Windows::Forms::Application::SetCompatibleTextRenderingDefault(false);
	Project1::MyForm form;
	System::Windows::Forms::Application::Run(% form);

	return 0;
}