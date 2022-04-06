#ifndef TEXTTABLE_H_
#define TEXTTABLE_H_

#include <iomanip>
#include <iostream>
#include <map>
#include <string>
#include <vector>

class TextTable {
public:
  enum class Alignment {
    LEFT,
    RIGHT
  }; // 强类型枚举 enum class 枚举类型名: 底层类型 {枚举值列表};
  typedef std::vector<std::string> Row;
  TextTable(char horizontal = '-', char vertical = '|', char corner = '+')
      : _horizontal(horizontal), _vertical(vertical), _corner(corner) {}

  void setAlignment(unsigned i, Alignment alignment) {
    _alignment[i] = alignment;
  }

  Alignment alignment(unsigned i) const { return _alignment[i]; }

  char horizontal() const { return _horizontal; }
  char vertical() const { return _vertical; }
  void add(const std::string &content) { _current.push_back(content); }
  void endOfRow() {
    _rows.push_back(_current);
    _current.assign(0, "");
  }

  template <typename Iterator> void addRow(Iterator begin, Iterator end) {
    for (auto i = begin; i != end; ++i) {
      add(*i);
    }
    endOfRow();
  }

  template <typename Container> void addRow(Container const &container) {
    addRow(container.begin(), container.end);
  }

  std::vector<Row> const &rows() const { return _rows; }

  void setup() const {
    determineWidths();
    setupAlignment();
  }

  std::string ruler() const {
    std::string result;
    result += _corner;
    for (auto width = _width.begin();width!= _width.end();++width) {
      result += repeat(*width, _horizontal);
      result += _corner;
    }
    return result;
  }

  int width(unsigned i) const {return _width[i];}

private:
  // +-----+-----+
  // |  A  |  B  |
  // +-----+-----+
  char _horizontal; // 横向分隔符
  char _vertical;   // 纵向分隔符
  char _corner;     // 边角分隔符

  Row _current;
  std::vector<Row> _rows;
  std::vector<unsigned> mutable _width;
  std::map<unsigned, Alignment> mutable _alignment;

  static std::string repeat(unsigned times, char c) {
    std::string result;
    for (; times > 0; --times) {
      result += c;
    }
    return result;
  }

  unsigned columns() const { return _rows[0].size(); } // 为什么是0

  // determine 判断
  void determineWidths() const {
    _width.assign(columns(), 0); // 赋 columns个值为0的元素到vector
    for (auto rowInterator = _rows.begin(); rowInterator != _rows.end();
         ++rowInterator) {
      const Row &row = *rowInterator;
      for (unsigned i = 0; i < row.size(); ++i) {
        _width[i] = _width[i] > row[i].size() ? _width[i] : row[i].size();
      }
    }
  }

  void setupAlignment() const {
    for (unsigned i = 0; i < columns(); ++i) {
      if (_alignment.find(i) == _alignment.end()) {
        _alignment[i] = Alignment::LEFT;
      }
    }
  }
};

std::ostream &operator<<(std::ostream &stream, TextTable const &table) noexcept{
  table.setup();
  stream << table.ruler() << "\n";
  for (auto rowIterator = table.rows().begin();rowIterator != table.rows().end();++ rowIterator) {
    TextTable::Row const &row = *rowIterator;
    for (unsigned i = 0;i < row.size();++i) {
      auto alignment = table.alignment(i)  == TextTable::Alignment::LEFT ? std::left : std::right;
      stream << std::setw(table.width(i)) << alignment << row[i];
      stream << table.vertical();
    }
    stream << "\n";
    stream << table.ruler() << "\n";
  }
  return stream;
}

#endif // TEXTTABLE_H_
