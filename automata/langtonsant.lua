win_title = "Langton's Ant"

cells_wide = 50
cells_high = 50

cell_size = 20
updates_per_second = 5

num_of_states = 10

state_colors = {
  "FFFFFF",
  "990000",
  "009900",
  "000099",
  "990099",
  "FF0000",
  "00FF00",
  "0000FF",
  "FF00FF",
  "000000"
}

get_neighbors = true;

function process(curr_state, neighbor_counts)
  local new_state = 0

  if curr_state == 0 or curr_state == 9 then
    if neighbor_counts[1] == 3 or neighbor_counts[1] == 6 then
      if curr_state == 0 then
        new_state = 4
      else
        new_state = 8
      end
    elseif neighbor_counts[3] == 1 or neighbor_counts[3] == 8 then
      if curr_state == 0 then
        new_state = 3
      else
        new_state = 7
      end
    elseif neighbor_counts[5] == 4 or neighbor_counts[5] == 5 then
      if curr_state == 0 then
        new_state = 2
      else
        new_state = 6
      end
    elseif neighbor_counts[7] == 2 or neighbor_counts[7] == 7 then
      if curr_state == 0 then
        new_state = 1
      else
        new_state = 5
      end
    else
      new_state = curr_state;
    end
  else
    if curr_state >= 5 then
      new_state = 0
    else
      new_state = 9
    end
  end

  return new_state
end
