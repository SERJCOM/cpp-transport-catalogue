#pragma once

#include <string>
#include <fstream>
#include "json.h"
#include "request_handler.h"


namespace ctlg{

bool Serialization(std::istream& in);

bool Deserialization(const json::Document& doc, ctlg::RequestHandler handler);

}