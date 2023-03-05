
int findRepeatNumber(vector<int> &nums)
{
    int ret = -1;
    int size = nums.size();
    for (int i = 0; i < size; ++i)
    {
        int original_value = nums[i] % size;
        if (nums[original_value] >= size)
        {
            ret = original_value;
            break;
        }
        else
        {
            nums[original_value] = nums[original_value] + size;
        }
    }
    return ret;
}


