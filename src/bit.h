#ifndef BIT_H
#define BIT_H

#define bit_set(flag, index) flag |= (1 << index)
#define bit_clear(flag, index) flag &= ~(1 << index)
#define bit_is_set(flag, index) (flag & (1 << index))
#define bit_is_clear(flag, index) (flag & (1 << index) == 0)

#endif // BIT_H
