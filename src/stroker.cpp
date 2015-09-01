#include <agge/stroker.h>

#include <agge/math.h>
#include <agge/stroke_features.h>

namespace agge
{
	struct stroke::point_ref
	{
		bool set_distance_to(const point_r &next)
		{
			distance = agge::distance(point, next);
			return distance > distance_epsilon;
		}

		point_r point;
		real_t distance;
	};

	stroke::stroke()
		: _cap(0), _join(0), _output_iterator(_output.end()), _state(0)
	{	}

	stroke::~stroke()
	{
		delete _cap;
		delete _join;
	}

	void stroke::remove_all()
	{
		_input.clear();
		_output.clear();
		_output_iterator = _output.end();
		_state = start;
	}

	void stroke::add_vertex(real_t x, real_t y, int command)
	{
		if (is_vertex(command))
		{
			bool add = true;
			point_ref p = { { x, y } };

			if (!_input.empty())
			{
				point_ref &last = *(_input.end() - 1);

				add = last.set_distance_to(p.point);
			}
			if (add)
				_input.push_back(p);
		}

		if (!(_state & ready) && _input.size() > 1)
			_state |= ready;

		if (agge::is_close(command))
			close();
	}
		
	int stroke::vertex(real_t *x, real_t *y)
	{
		for ( ; _state & ready; )
		{
			if (_output_iterator != _output.end())
			{
				*x = _output_iterator->x;
				*y = _output_iterator->y;
				++_output_iterator;
				int command = _state & moveto ? path_command_move_to : path_command_line_to;
				_state &= ~moveto;
				return command;
			}

			_output.clear();

			const input_vertices::const_iterator first = _input.begin();
			const input_vertices::const_iterator last = _input.end() - 1;
			const input_vertices::const_iterator prev = _i == first ? last : _i - 1;
			const input_vertices::const_iterator next = _i == last ? first : _i + 1;

			switch (_state & stage_mask)
			{
			case start:
				_i = first;
				set_state((is_closed() ? outline_forward : start_cap) | moveto);
				break;

			case start_cap:
				_cap->calc(_output, _width, _i->point, _i->distance, next->point);
				_i = next;
				set_state(_i == last ? end_cap : outline_forward);
				break;

			case outline_forward:
				_join->calc(_output, _width, prev->point, prev->distance, _i->point, _i->distance, next->point);
				_i = next;
				if (is_closed() && _i == first)
					set_state(end_poly1);
				else if (!is_closed() && _i == last)
					set_state(end_cap);
				break;

			case end_poly1:
				_output_iterator = _output.end();
				set_state(outline_backward | moveto);
				return path_command_end_poly | path_flag_close;

			case end_cap:
				_cap->calc(_output, _width, _i->point, prev->distance, prev->point);
				_i = prev;
				set_state(_i == first ? end_poly : outline_backward);
				break;

			case outline_backward:
				_join->calc(_output, _width, next->point, _i->distance, _i->point, prev->distance, prev->point);
				_i = prev;
				if (_i == first)
					set_state(end_poly);
				break;

			case end_poly:
				_output_iterator = _output.end();
				set_state(stop);
				return path_command_end_poly | path_flag_close;

			case stop:
				return path_command_stop;
			}

			_output_iterator = _output.begin();
		}
		return path_command_stop;
	}
		
	void stroke::width(real_t w)
	{	_width = 0.5f * w;	}

	bool stroke::is_closed() const
	{	return 0 != (_state & closed);	}

	void stroke::set_state(int stage_and_flags)
	{	_state = (_state & ~stage_mask) | stage_and_flags;	}

	void stroke::close()
	{
		const point_ref &first = *_input.begin();
		point_ref &last = *(_input.end() - 1);
		
		if (!last.set_distance_to(first.point))
			_input.pop_back();
		_state |= closed;
		if (_input.size() < 3)
			_state &= ~ready;
	}
}
