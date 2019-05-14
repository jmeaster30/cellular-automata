win_title = "My Game Of Life"
win_width = 800
win_height = 800
cell_size = 20
num_of_states = 2
kernal_width = 3
kernal_height = 3

function process(curr_state, n_counts)
  local new_state = 0

  if curr_state == 0 then
    if n_counts[1] == 3 then
      new_state = 1
    end
  else
    if n_counts[1] <= 1 then
      new_state = 0
    elseif n_counts[1] <= 4 then
      new_state = 0
    else
      new_state = 1
    end
  end

  return new_state
end
