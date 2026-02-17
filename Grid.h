#pragma once

#include <vector>

template<typename ValueType>
class Grid
{
public:
	Grid() : width(0)
	{}

	Grid(int x_size, int y_size, ValueType fill) :
		width(x_size),
		data(x_size*y_size, fill)
	{}

	Grid(const Grid& other) = default;
	Grid(Grid&& other) = default;
	Grid& operator=(const Grid& other) = default;
	Grid& operator=(Grid&& other) = default;

	int get_width() const { return width; }
	int get_height() const { return width <= 0 ? 0 : ((int)(data.size()))/width; }

	void fill(ValueType new_value)
	{
		for (ValueType& d : data)
		{
			d = new_value;
		}
	}

	ValueType& edit(int x, int y)
	{
		return data.at(x + y*width);
	}

	ValueType const& read(int x, int y) const
	{
		return data.at(x + y*width);
	}

	std::vector<ValueType> & edit_data()
	{
		return data;
	}

	std::vector<ValueType> const& read_data() const
	{
		return data;
	}

protected:
	int width = 0;
	std::vector<ValueType> data;
};
