win_title = "Brian's Brain"
win_width = 800
win_height = 800

cell_size = 20
num_of_states = 3

kernal_width = 3
kernal_height = 3

function process(curr_state, neighbor_counts)
  local new_state = 0

  if curr_state == 0 then
    if neighbor_counts[2] == 2 then
      new_state = 2
    else
      new_state = 0
    end
  elseif curr_state == 1 then
    new_state = 0
  elseif curr_state == 2 then
    new_state = 1
  end

  return new_state
end
