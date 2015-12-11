#include "Parser.h"
#include <vector>
#include <string>
#include <cstdio>
#include <iostream>
#include <cstdlib>

Parser::Parser(Lexer lexer, Tokenizer tokenizer) : lexer_tokens(lexer.get_tokens()),
    lexer_parser_pos(0), look(this->scan()), document_tokens(tokenizer.get_tokens()) {
        views.clear();
}

token Parser::scan() {
    if ((size_t)lexer_parser_pos < lexer_tokens.size())
        return lexer_tokens[lexer_parser_pos++];
    return token("", END);
}

void Parser::match(std::string str) {
    if (this->look == get_token(str))
        this->look = this->scan();
    else
        std::cout << "str:" << str << "look:" << this->look.value << "\n", this->error("match failed.");
}

void Parser::error(std::string str) {
    printf("%s\n", str.c_str());
    exit(2);
}

void Parser::output_view(view v, std::string alias_name) {
    // output view with view and its alias
    // if alias is EMPTY token means there is no name for the alias
    // todo
    std::cout << "output view: " << v.name << " " << alias_name << "\n";
}

void Parser::program() {
    token end_token = token("", END);
    while (true) {
        if (this->look == end_token)
            break;
        this->aql_stmt();
    }
}

void Parser::aql_stmt() {
    if (this->look.type == CREATE)
        this->create_stmt();
    else
        this->output_stmt();
    this->match(";");
}

void Parser::create_stmt() {
    this->match("create");
    this->match("view");
    std::string view_name = this->look.value;
    this->look = this->scan();
    this->match("as");
    std::vector<col> view_cols = this->view_stmt();
    // create view with name and cols
    view v = view(view_name);
    v.cols = view_cols;
    this->views.push_back(v);

    std::cout << "view name:" << view_name << "\n";
}

std::vector<col> Parser::view_stmt() {
    if (this->look.type == SELECT)
        return this->select_stmt();
    else
        return this->extract_stmt();
}

void Parser::output_stmt() {
    this->match("output");
    this->match("view");
    std::string output_view_name = this->look.value;
    this->look = this->scan();
    token alias_name = this->alias();
    for (int i = 0; (size_t)i < this->views.size(); i++)
        if (this->views[i].name == output_view_name) {
            output_view(this->views[i], alias_name.value);
            break;
        }
}

token Parser::alias() {
    if (this->look.type == AS) {
        token t = look;
        this->match("as");
        this->look = this->scan();
        return t;
    } else {
        return token("", EMPTY);
    }
}

std::vector<col> Parser::select_stmt() {
    this->match("select");
    std::vector<token> select_list_v = this->select_list();
    // repeat: ID ID (ID or not) if not, insert an EMPTY token
    this->match("from");
    std::vector<token> from_list_v = this->from_list();
    // repeat: ID ID
    // create cols with select_list_v and from_list_v and return a vector<col>
    // todo
    std::cout << "select_list_v:";
    for (int i = 0; (size_t)i < select_list_v.size(); i++)
        std::cout << " " << select_list_v[i].value;
    std::cout << "\n";
    std::cout << "from_list_v:";
    for (int i = 0; (size_t)i < from_list_v.size(); i++)
        std::cout << " " << from_list_v[i].value;
    std::cout << "\n";

    // todo
    std::vector<col> v;
    return v;
}

std::vector<token> Parser::select_list() {
    std::vector<token> select_list_v;
    while (true) {
        std::vector<token> select_item_v = this->select_item();
        select_list_v.insert(select_list_v.end(), select_item_v.begin(), select_item_v.end());
        if (this->look.type == COMMA)
            this->match(",");
        else
            break;
    }
    return select_list_v;
}

std::vector<token> Parser::select_item() {
    std::vector<token> select_item_v;
    select_item_v.push_back(this->look);
    this->look = this->scan();
    this->match(".");
    select_item_v.push_back(this->look);
    this->look = this->scan();
    select_item_v.push_back(this->alias());
    // if not id3 is EMPTY token
    return select_item_v;
}

std::vector<token> Parser::from_list() {
    std::vector<token> from_list_v;
    while (true) {
        std::vector<token> from_item_v = this->from_item();
        from_list_v.insert(from_list_v.end(), from_item_v.begin(), from_item_v.end());
        if (this->look.type == COMMA)
            this->match(",");
        else
            break;
    }
    return from_list_v;
}

std::vector<token> Parser::from_item() {
    std::vector<token> from_item_v;
    from_item_v.push_back(this->look);
    this->look = this->scan();
    from_item_v.push_back(this->look);
    this->look = this->scan();
    return from_item_v;
}

std::vector<col> Parser::extract_stmt() {
    this->match("extract");
    std::vector<token> extract_spec_v = this->extract_spec();
    // extract_spec[0] == EMPTY token -> regex_spec else pattern_spec
    this->match("from");
    std::vector<token> from_list_v = this->from_list();
    // create cols and return vector<col>
    // todo
    std::cout << "extract_spec_v:";
    for (int i = 0; (size_t)i < extract_spec_v.size(); i++)
        std::cout << " " << extract_spec_v[i].value;
    std::cout << "\n";
    std::cout << "from_list_v:";
    for (int i = 0; (size_t)i < from_list_v.size(); i++)
        std::cout << " " << from_list_v[i].value;
    std::cout << "\n";

    //todo
    std::vector<col> v;
    return v;
}

std::vector<token> Parser::extract_spec() {
    if (this->look.type == REGEX)
        return this->regex_spec();
    else
        return this->pattern_spec();
}

std::vector<token> Parser::regex_spec() {
    std::vector<token> regex_spec_v;
    regex_spec_v.push_back(token("", EMPTY));
    this->match("regex");
    regex_spec_v.push_back(this->look);
    this->look = this->scan();
    this->match("on");
    std::vector<token> column_v = this->column();
    regex_spec_v.insert(regex_spec_v.end(), column_v.begin(), column_v.end());
    std::vector<token> name_spec_v = this->name_spec();
    regex_spec_v.insert(regex_spec_v.end(), name_spec_v.begin(), name_spec_v.end());
    return regex_spec_v;
    // EMPTY + REG + column_v(ID ID) + name_spec_v
}

std::vector<token> Parser::column() {
    std::vector<token> column_v;
    column_v.push_back(this->look);
    this->look = this->scan();
    this->match(".");
    column_v.push_back(this->look);
    this->look = this->scan();
    return column_v;
}

std::vector<token> Parser::name_spec() {
    std::vector<token> name_spec_v;
    if (this->look.type == AS)
        this->match("as"), name_spec_v.push_back(this->look), this->look = this->scan();
    else
        this->match("return"), name_spec_v = this->group_spec();
    return name_spec_v;
    // if as -> only 1 token
    // else return group
}

std::vector<token> Parser::group_spec() {
    std::vector<token> group_spec_v;
    while (true) {
        std::vector<token> single_group_v = this->single_group();
        group_spec_v.insert(group_spec_v.end(), single_group_v.begin(), single_group_v.end());
        if (this->look.type == AND)
            this->match("and");
        else
            break;
    }
    return group_spec_v;
}

std::vector<token> Parser::single_group() {
    std::vector<token> single_group_v;
    this->match("group");
    single_group_v.push_back(this->look);
    this->look = this->scan();
    this->match("as");
    single_group_v.push_back(this->look);
    this->look = this->scan();
    return single_group_v;
}

std::vector<token> Parser::pattern_spec() {
    std::vector<token> pattern_spec_v;
    this->match("pattern");
    std::vector<token> pattern_expr_v = this->pattern_expr();
    pattern_spec_v.insert(pattern_spec_v.end(), pattern_expr_v.begin(), pattern_expr_v.end());
    std::vector<token> name_spec_v = this->name_spec();
    pattern_spec_v.insert(pattern_spec_v.end(), name_spec_v.begin(), name_spec_v.end());
    return pattern_spec_v;
}

std::vector<token> Parser::pattern_expr() {
    std::vector<token> pattern_expr_v;
    while (true) {
        std::vector<token> pattern_pkg_v = this->pattern_pkg();
        pattern_expr_v.insert(pattern_expr_v.end(), pattern_pkg_v.begin(), pattern_pkg_v.end());
        if (this->look.type == LEFTBRACKET || this->look.type == TOKEN || this->look.type == REG || this->look.type == LESSTHAN)
            continue;
        else
            break;
    }
    return pattern_expr_v;
}

std::vector<token> Parser::pattern_pkg() {
    if (this->look.type == LEFTBRACKET)
        return this->pattern_group();
    std::vector<token> pattern_pkg_v;
    std::vector<token> atom_v = this->atom();
    pattern_pkg_v.insert(pattern_pkg_v.end(), atom_v.begin(), atom_v.end());
    if (this->look.type == CURLYLEFTBRACKET)
        this->match("{"), pattern_pkg_v.push_back(this->look), this->look = this->scan(), this->match(","),
            pattern_pkg_v.push_back(this->look), this->look = this->scan(), this->match("}");
    return pattern_pkg_v;
}

std::vector<token> Parser::atom() {
    std::vector<token> atom_v;
    if (this->look.type == REG)
        atom_v.push_back(this->look), this->look = this->scan();
    else {
        this->match("<");
        if (this->look.type == TOKEN)
            atom_v.push_back(this->look), this->look = this->scan();
        else {
            std::vector<token> column_v = this->column();
            atom_v.insert(atom_v.end(), column_v.begin(), column_v.end());
        }
        this->match(">");
    }
    return atom_v;
}

std::vector<token> Parser::pattern_group() {
    this->match("(");
    std::vector<token> pattern_group_v = this->pattern_expr();
    this->match(")");
    return pattern_group_v;
}

