win_title = "Conway's Game Of Life"

function process(curr_state, neighbor_counts)
  local new_state = 0

  if curr_state == 0 then
    if neighbor_counts[1] == 3 then
      new_state = 1
    end
  else
    if neighbor_counts[1] <= 1 then
      new_state = 0
    elseif neighbor_counts[1] >= 4 then
      new_state = 0
    else
      new_state = 1
    end
  end

  return new_state
end