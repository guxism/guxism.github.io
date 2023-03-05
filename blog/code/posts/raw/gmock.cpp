



class Rng
{
  public:
    virtual ~Rng() {}
    virtual double generate(double min, double max) = 0;
};

class CoinFlipper
{
    Rng *d_rng; // held, not owned

  public:
    enum Result
    {
        HEADS = 0,
        TAILS = 1
    };
    CoinFlipper(Rng *rng) : d_rng(rng) {}
    Result flipCoin() const
    {
        const double val = d_rng->generate(0.0, 1.0);
        return (val < 0.5) ? HEADS : TAILS;
    };
};

#include <gmock/gmock.h>
#include <gtest/gtest.h>
class MockRng : public Rng
{
  public:
    MOCK_METHOD2(generate, double(double, double));
};

using ::testing::AtLeast;
using ::testing::DoubleEq;
using ::testing::Exactly;
using ::testing::Return;

TEST(CoinFlipper, ShouldReturnHeadsIfRandValueIsLessThanProbability)
{
    MockRng rng;

    EXPECT_CALL(rng, generate(DoubleEq(0.0), DoubleEq(1.0)))
        .Times(Exactly(1))
        .WillOnce(Return(0.25));

    CoinFlipper coinFlipper(&rng);

    CoinFlipper::Result result = coinFlipper.flipCoin();

    EXPECT_EQ(CoinFlipper::HEADS, result);

}



class CoinFlipperTest : public testing::Test,
                        public testing::WithParamInterface<
                            std::pair<const double, const CoinFlipper::Result>>
{
};

TEST_P(CoinFlipperTest, CoinFlip)
{
    const double randomVal = GetParam().first;
    const CoinFlipper::Result expectedResult = GetParam().second;

    MockRng rng;
    EXPECT_CALL(rng, generate(DoubleEq(0.0), DoubleEq(1.0)))
        .Times(Exactly(1))
        .WillOnce(Return(randomVal));

    CoinFlipper coinFlipper(&rng);
    CoinFlipper::Result result = coinFlipper.flipCoin();

    EXPECT_EQ(expectedResult, result);
}

using ::testing::Values;
INSTANTIATE_TEST_CASE_P(ValidRandomNumberGenerated, CoinFlipperTest,
                        Values(std::make_pair(0.0, CoinFlipper::HEADS),
                               std::make_pair(0.25, CoinFlipper::HEADS),
                               std::make_pair(0.49999, CoinFlipper::HEADS),
                               std::make_pair(0.5, CoinFlipper::TAILS),
                               std::make_pair(0.75, CoinFlipper::TAILS),
                               std::make_pair(1.0, CoinFlipper::TAILS)));
int main(int argc, char **argv)
{
    ::testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}

