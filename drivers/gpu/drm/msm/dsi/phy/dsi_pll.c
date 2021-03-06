// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2012-2015, The Linux Foundation. All rights reserved.
 */

#include "dsi_phy.h"
#include "dsi_pll.h"

/*
 * DSI PLL Helper functions
 */
long msm_dsi_pll_helper_clk_round_rate(struct clk_hw *hw,
		unsigned long rate, unsigned long *parent_rate)
{
	struct msm_dsi_pll *pll = hw_clk_to_pll(hw);

	if      (rate < pll->cfg->min_pll_rate)
		return  pll->cfg->min_pll_rate;
	else if (rate > pll->cfg->max_pll_rate)
		return  pll->cfg->max_pll_rate;
	else
		return rate;
}

int msm_dsi_pll_helper_clk_prepare(struct clk_hw *hw)
{
	struct msm_dsi_pll *pll = hw_clk_to_pll(hw);
	int ret = 0;

	/*
	 * Certain PLLs do not allow VCO rate update when it is on.
	 * Keep track of their status to turn on/off after set rate success.
	 */
	if (unlikely(pll->pll_on))
		return 0;

	ret = pll->cfg->pll_ops.enable_seq(pll);
	if (ret) {
		DRM_ERROR("DSI PLL failed to lock\n");
		return ret;
	}

	pll->pll_on = true;

	return 0;
}

void msm_dsi_pll_helper_clk_unprepare(struct clk_hw *hw)
{
	struct msm_dsi_pll *pll = hw_clk_to_pll(hw);

	if (unlikely(!pll->pll_on))
		return;

	pll->cfg->pll_ops.disable_seq(pll);

	pll->pll_on = false;
}

void msm_dsi_pll_helper_unregister_clks(struct clk **clks, u32 num_clks)
{
	if (!num_clks || !clks)
		return;

	do {
		clk_unregister(clks[--num_clks]);
		clks[num_clks] = NULL;
	} while (num_clks);
}

/*
 * DSI PLL API
 */
void msm_dsi_pll_destroy(struct msm_dsi_pll *pll)
{
	if (pll->cfg->pll_ops.destroy)
		pll->cfg->pll_ops.destroy(pll);
}

void msm_dsi_pll_save_state(struct msm_dsi_pll *pll)
{
	if (pll->cfg->pll_ops.save_state) {
		pll->cfg->pll_ops.save_state(pll);
		pll->state_saved = true;
	}
}

int msm_dsi_pll_restore_state(struct msm_dsi_pll *pll)
{
	int ret;

	if (pll->cfg->pll_ops.restore_state && pll->state_saved) {
		ret = pll->cfg->pll_ops.restore_state(pll);
		if (ret)
			return ret;

		pll->state_saved = false;
	}

	return 0;
}
