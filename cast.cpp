
int main()
{
    {
        int var = 0;
        void *ptr = reinterpret_cast<void *>(var);
        // void *ptr2 = static_cast<void *>(var); //error
    }
    return 0;
}